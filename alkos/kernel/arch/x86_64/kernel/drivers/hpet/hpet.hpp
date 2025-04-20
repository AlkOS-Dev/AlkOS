#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_

#include <uacpi/acpi.h>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>

class Hpet final
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    explicit Hpet(acpi_hpet* table);

    // ------------------------------
    // Class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    acpi_gas address_;
    u8 num_comparators_;
    bool is_comparator_64_bit_;
    u16 ticks_;
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
