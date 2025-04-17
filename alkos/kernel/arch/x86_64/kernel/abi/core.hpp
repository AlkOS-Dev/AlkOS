#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_

#include "core.hpp"

namespace arch
{
class Core : public CoreABI
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Core() = delete;

    explicit Core(u64 acpi_id, u64 apic_id);
    ~Core() = default;

    // ------------------------------
    // ABI implementation
    // ------------------------------

    void EnableCore();

    // ------------------------------
    // Class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    u64 acpi_id_;
    u64 apic_id_;
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_
