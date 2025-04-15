#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_

#include <extensions/types.hpp>

class IoApic final
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    IoApic(u8 id, u32 address, u32 gsi_base);

    ~IoApic() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    u8 id_;
    u32 address_;
    u32 gsi_base_;
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_
