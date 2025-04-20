#include <drivers/hpet/hpet.hpp>

Hpet::Hpet(acpi_hpet *table)
{
    ASSERT_NOT_NULL(table);

    address_ = table->address;

    /* Load the HPET capabilities */
    // const auto capabilities =

    TRACE_INFO(
        "Initialized driver for HPET: "
        "address: %016X, "
        "num_comparators: %u, "
        "is_comparator_64_bit: %u, "
        "ticks: %u",
        address_.address, num_comparators_, is_counter_32_bit_, ticks_
    );
}
