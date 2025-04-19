#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_

#include <drivers/apic/local_apic.hpp>

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

    explicit Core(u32 acpi_id, u32 apic_id);
    ~Core() = default;

    // ------------------------------
    // ABI implementation
    // ------------------------------

    void EnableCore();

    NODISCARD FORCE_INLINE_F u32 GetCoreId() const { return apic_id_; }

    // ------------------------------
    // Class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    u32 acpi_id_;
    u32 apic_id_;
};

NODISCARD WRAP_CALL u32 GetCurrentCoreId() { return LocalApic::GetCoreId(); }

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_
