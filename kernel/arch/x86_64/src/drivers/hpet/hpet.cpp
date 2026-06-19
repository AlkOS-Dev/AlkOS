// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "drivers/hpet/hpet.hpp"
#include "cpu/utils.hpp"
#include "hal/interrupt_params.hpp"
#include "modules/hardware.hpp"

// ------------------------------
// Clock callbacks
// ------------------------------

NODISCARD FAST_CALL Hpet *GetHpetFromClockEntry(hardware::ClockRegistryEntry *clock_entry)
{
    ASSERT_NOT_NULL(clock_entry);
    auto hpet = static_cast<Hpet *>(clock_entry->own_data);
    ASSERT_NOT_NULL(hpet);
    return hpet;
}

static u64 ReadCb(hardware::ClockRegistryEntry *clock_entry)
{
    return GetHpetFromClockEntry(clock_entry)->ReadMainCounter();
}

static bool EnableDeviceCb(hardware::ClockRegistryEntry *clock_entry)
{
    GetHpetFromClockEntry(clock_entry)->StartMainCounter();
    return true;
}

static bool DisableDeviceCb(hardware::ClockRegistryEntry *clock_entry)
{
    GetHpetFromClockEntry(clock_entry)->StopMainCounter();
    return true;
}

static void StopCounterCb(hardware::ClockRegistryEntry *clock_entry)
{
    GetHpetFromClockEntry(clock_entry)->StopMainCounter();
}

static void ResumeCounterCb(hardware::ClockRegistryEntry *clock_entry)
{
    GetHpetFromClockEntry(clock_entry)->StartMainCounter();
}

// ------------------------------
// Event Clock callbacks
// ------------------------------

static void OnEntryCb(hardware::EventClockRegistryEntry *clock_entry)
{
    if (clock_entry->state == hardware::EventClockState::kOneshot) {
        clock_entry->state = hardware::EventClockState::kOneshotIdle;
    }
}

static u32 NextEventCb(hardware::EventClockRegistryEntry *entry, const u64 time_ns)
{
    if (entry->state == hardware::EventClockState::kDisabled) {
        return 1;
    }

    const auto hpet = static_cast<Hpet *>(entry->own_data);
    hpet->DisableTimer(0);

    const u64 hw_irq =
        HardwareModule::Get()
            .GetInterrupts()
            .GetLit()
            .TranslateToHw<intr::InterruptType::kHardwareInterrupt>(hal::kTimerHwLirq);
    const u64 time_femto = time_ns * Hpet::kNsToFemto;

    if (entry->state == hardware::EventClockState::kPeriodic) {
        hpet->SetupPeriodicTimer(0, time_femto, hw_irq);
    } else {
        entry->state = hardware::EventClockState::kOneshot;
        hpet->SetupOneShotTimer(0, time_femto, hw_irq);
    }

    return 0;
}

static u32 SetPeriodicCb(hardware::EventClockRegistryEntry *entry)
{
    if (entry->state != hardware::EventClockState::kPeriodic) {
        const auto hpet = static_cast<Hpet *>(entry->own_data);

        if (!hpet->IsTimerSupportingPeriodic(0)) {
            DEBUG_WARN_TIME("Hpet does not support periodic!");
            return 1;
        }

        hpet->DisableTimer(0);
        entry->state = hardware::EventClockState::kPeriodic;
    }

    return 0;
}

static u32 SetOneshotCb(hardware::EventClockRegistryEntry *entry)
{
    if (entry->state != hardware::EventClockState::kOneshot &&
        entry->state != hardware::EventClockState::kOneshotIdle) {
        const auto hpet = static_cast<Hpet *>(entry->own_data);
        hpet->DisableTimer(0);
        entry->state = hardware::EventClockState::kOneshotIdle;
    }

    return 0;
}

// ------------------------------
// Implementations
// ------------------------------

Hpet::Hpet(acpi_hpet *table)
{
    ASSERT_NOT_NULL(table);
    address_ = table->address;

    DEBUG_INFO_INTERRUPTS(
        "HPET address: "
        "access_size: %hhu "
        "address_space_id: %hhu "
        "register_bit_offset: %hhu "
        "register_bit_width: %hhu",
        address_.access_size, address_.address_space_id, address_.register_bit_offset,
        address_.register_bit_width
    );

    TODO_WHEN_VMEM_WORKS
    /* TODO: Map physical address to virtual */

    /* Load the HPET capabilities */
    const auto capabilities = ReadRegister<GeneralCapabilitiesAndIdReg>(kGeneralCapabilitiesRegRO);
    num_comparators_        = capabilities.num_comparators + 1;  // Number of comparators is 0-based
    is_counter_32_bit_ = capabilities.timer_size == GeneralCapabilitiesAndIdReg::TimerSize::k32Bit;
    clock_period_      = capabilities.clock_period;
    is_legacy_mode_available_ =
        capabilities.timer_type == GeneralCapabilitiesAndIdReg::TimerType::kLegacyReplacement;

    /* According to spec must not be 00h */
    ASSERT_NOT_ZERO(capabilities.revision_id);

    DEBUG_INFO_INTERRUPTS(
        "Initialized driver for HPET: "
        "address: %016llX, "
        "vendor_id: %04X, "
        "revision: %08X, "
        "num_comparators: %u, "
        "is_counter_32_bit_: %u, "
        "is_legacy_mode_available_: %u, "
        "clock_period: %u femtoseconds",
        address_.address, capabilities.vendor_id, capabilities.revision_id, num_comparators_,
        is_counter_32_bit_, capabilities.timer_type, clock_period_
    );

    /* Gather information about comparators to avoid reading registers each time */
    for (u32 timer_idx = 0; timer_idx < num_comparators_; ++timer_idx) {
        const auto conf_reg =
            ReadRegister<TimerConfigurationReg>(GetTimerConfigurationRegRW(timer_idx));

        const bool supports_64_bit =
            conf_reg.is_64_bit_comparator == TimerConfigurationReg::Is64BitComparator::k64Bit;
        comparators_64bit_supported_.Set(timer_idx, supports_64_bit);

        const bool supports_periodic =
            conf_reg.periodic_supported == TimerConfigurationReg::PeriodicSupported::kSupported;
        comparators_periodic_supported_.Set(timer_idx, supports_periodic);

        comparators_allowed_irqs_[timer_idx] = conf_reg.route_capabilities;

        DEBUG_INFO_INTERRUPTS(
            "Comp %u 64-bit capable: %u, periodic capable: %u, allowed irqs: %08X", timer_idx,
            static_cast<u32>(supports_64_bit), static_cast<u32>(supports_periodic),
            conf_reg.route_capabilities.ToU32()
        );
    }

    /* Ensure sensible initial state */
    SetupStandardMapping();
    StopMainCounter();

    // ========================
    // Register as clock
    hardware::ClockRegistryEntry hpet_entry = {};

    /* Clock data */
    hpet_entry.id            = static_cast<u64>(arch::HardwareClockId::kHpet);
    hpet_entry.frequency_kHz = (kFemtoSecondsPerSecond / clock_period_) / 1000;

    /* Callbacks */
    hpet_entry.read           = ReadCb;
    hpet_entry.enable_device  = EnableDeviceCb;
    hpet_entry.disable_device = DisableDeviceCb;
    hpet_entry.stop_counter   = StopCounterCb;
    hpet_entry.resume_counter = ResumeCounterCb;

    hpet_entry.clock_numerator = clock_period_;
    hpet_entry.clock_denominator =
        1'000'000;  // Convert femtoseconds (10^-15) to nanoseconds (10^-9)

    /* Own data */
    hpet_entry.own_data = this;

    HardwareModule::Get().GetClockRegistry().Register(hpet_entry);

    // ========================
    // Register as event clock
    hardware::EventClockRegistryEntry hpet_event_entry{};

    hpet_event_entry.id                 = static_cast<u64>(arch::HardwareEventClockId::kHpet);
    hpet_event_entry.state              = hardware::EventClockState::kDisabled;
    hpet_event_entry.flags.IsCoreLocal  = false;
    hpet_event_entry.next_event_time_ns = 0;
    hpet_event_entry.own_data           = this;

    hpet_event_entry.supported_cores.SetAll(true);
    hpet_event_entry.min_next_event_time_ns = clock_period_ / 1'000'000;

    hpet_event_entry.cbs.next_event   = NextEventCb;
    hpet_event_entry.cbs.set_oneshot  = SetOneshotCb;
    hpet_event_entry.cbs.set_periodic = SetPeriodicCb;
    hpet_event_entry.cbs.on_entry     = OnEntryCb;
    hpet_event_entry.cbs.on_exit      = nullptr;

    HardwareModule::Get().GetEventClockRegistry().Register(hpet_event_entry);
}

u64 Hpet::AdjustTimeToHpetCapabilities(const u64 time_femto) const
{
    if (time_femto > clock_period_) {
        return (time_femto + clock_period_ / 2) / clock_period_;
    }

    // TODO: increase accuracy, important when hpet resolution is around 100ns etc
    return 1;
}
