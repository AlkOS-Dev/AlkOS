#include <drivers/hpet/hpet.hpp>

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
    ticks_             = capabilities.clock_period;

    TRACE_INFO(
        "Initialized driver for HPET: "
        "address: %016X, "
        "num_comparators: %u, "
        "is_counter_32_bit_: %u, "
        "ticks: %u femtoseconds",
        address_.address, num_comparators_, is_counter_32_bit_, ticks_
    );

    /* Gather information about comparators to avoid reading registers each time */
    for (u32 timer_idx = 0; timer_idx < num_comparators_; ++timer_idx) {
        const bool supports_64_bit =
            ReadRegister<TimerConfigurationReg>(GetTimerConfigurationRegRW(timer_idx))
                .is_64_bit_comparator == TimerConfigurationReg::Is64BitComparator::k64Bit;
        comparators_64bit_supported_.Set(timer_idx, supports_64_bit);

        const bool supports_periodic =
            ReadRegister<TimerConfigurationReg>(GetTimerConfigurationRegRW(timer_idx))
                .periodic_supported == TimerConfigurationReg::PeriodicSupported::kSupported;
        comparators_periodic_supported_.Set(timer_idx, supports_periodic);

        TRACE_DEBUG(
            "Comp %u 64-bit capable: %u, periodic capable: %u", timer_idx,
            static_cast<u32>(supports_64_bit), static_cast<u32>(supports_periodic)
        );
    }
}

void Hpet::Setup() {}

u64 Hpet::AdjustTimeToHpetCapabilities(const u64 time_femto) const
{
    if (time_femto > ticks_) {
        return time_femto;
    }

    // TODO: increase accuracy, important when hpet resolution is around 100ns etc
    return ticks_;
}
