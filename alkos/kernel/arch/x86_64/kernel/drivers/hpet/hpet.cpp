#include "drivers/hpet/hpet.hpp"
#include "arch_utils.hpp"
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
// Implementations
// ------------------------------

Hpet::Hpet(acpi_hpet *table)
{
    ASSERT_NOT_NULL(table);
    address_ = table->address;

    TRACE_DEBUG(
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

    TRACE_INFO(
        "Initialized driver for HPET: "
        "address: %016X, "
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

        TRACE_DEBUG(
            "Comp %u 64-bit capable: %u, periodic capable: %u, allowed irqs: %08X", timer_idx,
            static_cast<u32>(supports_64_bit), static_cast<u32>(supports_periodic),
            conf_reg.route_capabilities.ToU32()
        );
    }

    /* Ensure sensible initial state */
    SetupStandardMapping();
    StopMainCounter();

    hardware::ClockRegistryEntry hpet_entry = {};

    /* Clock data */
    hpet_entry.id            = static_cast<u64>(arch::HardwareClockId::kHpet);
    hpet_entry.frequency_kHz = (kFemtoSecondsPerSecond / clock_period_) / 1000;

    /* According to spec for intervals > 1ms, minimum requirement for clock drift is 0.05% */
    hpet_entry.ns_uncertainty_margin_per_sec = kNanosInSecond / 2000;  // 0.05% of 1 seconds
    hpet_entry.clock_numerator               = kFemtoSecondsPerSecond;
    hpet_entry.clock_denominator             = clock_period_;

    /* Callbacks */
    hpet_entry.read           = ReadCb;
    hpet_entry.enable_device  = EnableDeviceCb;
    hpet_entry.disable_device = DisableDeviceCb;
    hpet_entry.stop_counter   = StopCounterCb;
    hpet_entry.resume_counter = ResumeCounterCb;

    /* Own data */
    hpet_entry.own_data = this;

    HardwareModule::Get().GetClockRegistry().Register(hpet_entry);
}

u64 Hpet::AdjustTimeToHpetCapabilities(const u64 time_femto) const
{
    if (time_femto > clock_period_) {
        return (time_femto + clock_period_ / 2) / clock_period_;
    }

    // TODO: increase accuracy, important when hpet resolution is around 100ns etc
    return 1;
}
