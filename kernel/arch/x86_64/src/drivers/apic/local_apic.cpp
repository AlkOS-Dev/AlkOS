// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "drivers/apic/local_apic.hpp"

#include <assert.h>
#include <todo.hpp>

#include <acpi/acpi.hpp>
#include <mem/types.hpp>
#include <modules/hardware.hpp>
#include <trace_framework.hpp>

#include "drivers/pic8259/pic8259.hpp"

// ------------------------------
// Static functions
// ------------------------------

static void ApplyNmiSource_(const acpi_madt_lapic_nmi *nmi_source)
{
    ASSERT_NOT_NULL(nmi_source);

    TRACE_INFO_INTERRUPTS(
        "Got LAPIC NMI source: "
        "flags: %hu, "
        "lapic_id: %hhu, "
        "lint: %hhu",
        nmi_source->flags, nmi_source->uid, nmi_source->lint
    );

    ASSERT_LT(nmi_source->lint, 2, "LINT number is out of range (0-1)");
    const u32 reg_offset =
        nmi_source->lint == 0 ? LocalApic::kLvtLint0RegRW : LocalApic::kLvtLint1RegRW;

    auto reg          = LocalApic::ReadRegister<LocalApic::LocalVectorTableRegister>(reg_offset);
    reg.delivery_mode = LocalApic::LocalVectorTableRegister::DeliveryMode::kNMI;
    LocalApic::WriteRegister(reg_offset, reg);
}

static void ParseMadtRules_()
{
    auto table = ACPI::GetTable<acpi_madt>();
    R_ASSERT_TRUE(table.IsValid(), "MADT table is not found, only platform with apic supported...");

    table.ForEachTableEntry([](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_lapic_nmi>(entry);

        if (table_ptr == nullptr) {
            return;
        }

        ApplyNmiSource_(table_ptr);
    });
}

// ------------------------------
// Driver functions
// ------------------------------

static void LApicACK(intr::LitHwEntry &) { LocalApic::SendEOI(); }

static u32 NextEventCb(hardware::EventClockRegistryEntry *entry, const u64 time_ns)
{
    static constexpr u32 kTimerDivider = 8;

    if (entry->state == hardware::EventClockState::kDisabled) {
        return 1;
    }

    const u64 lapic_freq   = static_cast<LocalApic *>(entry->own_data)->GetFreqHz();
    const u64 divided_freq = lapic_freq / kTimerDivider;

    LocalApic::DisableTimer();
    LocalApic::SetTimerDivider(kTimerDivider);

    LocalApic::LocalVectorTableTimerRegister reg{};
    reg.mask       = LocalApic::LocalVectorTableTimerRegister::Mask::kEnabled;
    reg.timer_mode = entry->state == hardware::EventClockState::kPeriodic
                         ? LocalApic::LocalVectorTableTimerRegister::TimerMode::kPeriodic
                         : LocalApic::LocalVectorTableTimerRegister::TimerMode::kOneShot;
    reg.vector     = arch::kTimerHwInt;

    LocalApic::WriteRegister(LocalApic::kLvtTimerRegRW, reg);

    u64 ticks = (time_ns * divided_freq) / kNanosInSecond;
    if (ticks == 0)
        ticks = 1;

    LocalApic::SetTimerCounter(static_cast<u32>(ticks));

    return 0;
}

static u32 SetPeriodicCb(hardware::EventClockRegistryEntry *entry)
{
    if (entry->state != hardware::EventClockState::kPeriodic) {
        LocalApic::DisableTimer();
        entry->state = hardware::EventClockState::kPeriodic;
    }

    return 0;
}

static u32 SetOneshotCb(hardware::EventClockRegistryEntry *entry)
{
    if (entry->state != hardware::EventClockState::kOneshot &&
        entry->state != hardware::EventClockState::kOneshotIdle) {
        LocalApic::DisableTimer();
        entry->state = hardware::EventClockState::kOneshotIdle;
    }

    return 0;
}

// ------------------------------
// Implementations
// ------------------------------

LocalApic::LocalApic()
{
    driver_.name    = "Local APIC";
    driver_.data    = this;
    driver_.cbs.ack = LApicACK;
}

void LocalApic::Enable()
{
    R_ASSERT_TRUE(IsSupported(), "APIC is not supported on this platform...");

    DEBUG_INFO_INTERRUPTS("Assuming APIC address as: %016llX", local_apic_physical_address_);

    /* Enable Local Apic by ENABLE flag added to address (Might be enabled or might be not) */
    is_enabled_ = true;
    SetPhysicalAddressOnCore(local_apic_physical_address_);

    DEBUG_INFO_INTERRUPTS("Configuring LAPIC for core with id: %u", GetCoreId());

    /* Configure apic based on MADT entries */
    ParseMadtRules_();

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    auto reg    = ReadRegister<SpuriousInterruptRegister>(kSpuriousInterruptRegRW);
    reg.enabled = true;
    reg.vector  = kSpuriousVector;

    WriteRegister(kSpuriousInterruptRegRW, reg);
    WriteRegister(kTaskPriorityRegRW, 0);

    DEBUG_INFO_INTERRUPTS("Local APIC enabled...");
}
void LocalApic::RegisterAsEventClock()
{
    timer_freq_hz_ = MeasureFreqHz_();

    hardware::EventClockRegistryEntry lapic_entry{};

    lapic_entry.id                 = static_cast<u64>(arch::HardwareEventClockId::kLapic);
    lapic_entry.state              = hardware::EventClockState::kDisabled;
    lapic_entry.flags.IsCoreLocal  = true;
    lapic_entry.next_event_time_ns = 0;
    lapic_entry.own_data           = this;

    lapic_entry.supported_cores.SetAll(true);
    lapic_entry.min_next_event_time_ns = kNanosInSecond / timer_freq_hz_;

    lapic_entry.cbs.next_event   = NextEventCb;
    lapic_entry.cbs.set_oneshot  = SetOneshotCb;
    lapic_entry.cbs.set_periodic = SetPeriodicCb;
    lapic_entry.cbs.on_entry     = nullptr;
    lapic_entry.cbs.on_exit      = nullptr;

    HardwareModule::Get().GetEventClockRegistry().Register(lapic_entry);
}

u64 LocalApic::MeasureFreqHz_()
{
    static constexpr u64 kCalibrationTimeMs = 200;

    const u64 hw_irq =
        HardwareModule::Get()
            .GetInterrupts()
            .GetLit()
            .TranslateToHw<intr::InterruptType::kHardwareInterrupt>(hal::kTimerHwLirq);

    LocalVectorTableTimerRegister enabled_reg{};
    enabled_reg.vector     = hw_irq;
    enabled_reg.timer_mode = LocalVectorTableTimerRegister::TimerMode::kOneShot;
    enabled_reg.mask       = LocalVectorTableTimerRegister::Mask::kEnabled;

    LocalVectorTableTimerRegister disabled_reg = enabled_reg;
    disabled_reg.mask                          = LocalVectorTableTimerRegister::Mask::kDisabled;

    SetTimerDivider(1);
    SetTimerCounter(-1);
    WriteRegister(kLvtTimerRegRW, enabled_reg);
    const u64 lapic_freq_hz = HardwareModule::Get().GetInterrupts().GetHpet()->CalibrateByHpetHz(
        GetTimerCounter, kCalibrationTimeMs
    );
    WriteRegister(kLvtTimerRegRW, disabled_reg);

    DEBUG_INFO_INTERRUPTS("Lapic Timer frequency in Hz: %llu", lapic_freq_hz);
    return lapic_freq_hz;
}
