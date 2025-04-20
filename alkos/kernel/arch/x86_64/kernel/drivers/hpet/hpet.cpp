#include <drivers/hpet/hpet.hpp>

Hpet::Hpet(acpi_hpet *table)
{
    ASSERT_NOT_NULL(table);

    address_         = table->address;
    num_comparators_ = (table->block_id >> ACPI_HPET_NUMBER_OF_COMPARATORS_SHIFT) &
                       ACPI_HPET_NUMBER_OF_COMPARATORS_MASK;
    is_comparator_64_bit_ = true;
    ticks_                = table->min_clock_tick;

    TRACE_INFO(
        "Initialized driver for HPET: "
        "address: %016X, "
        "num_comparators: %u, "
        "is_comparator_64_bit: %u, "
        "ticks: %u",
        address_.address, num_comparators_, is_comparator_64_bit_, ticks_
    );
}
