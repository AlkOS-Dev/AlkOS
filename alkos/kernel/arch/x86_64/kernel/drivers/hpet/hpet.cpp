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
        "ticks: %u",
        address_.address, num_comparators_, is_counter_32_bit_, ticks_
    );
}

void Hpet::Enable() {}

void Hpet::Disable() {}
