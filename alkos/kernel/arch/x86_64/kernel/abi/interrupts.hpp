#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_INTERRUPTS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_INTERRUPTS_HPP_

#include "acpi/acpi.hpp"
#include "drivers/apic/io_apic.hpp"
#include "interrupts/idt.hpp"
#include "todo.hpp"

namespace arch
{
class Interrupts : public InterruptsABI
{
    public:
    Interrupts()  = default;
    ~Interrupts() = default;

    // ------------------------------
    // ABI
    // ------------------------------

    void Initialise();

    // ------------------------------
    // Class methods
    // ------------------------------

    /* First stage init is used to allow correct exception handling in whole memory initialization
     * code */
    void FirstStageInit();

    void AllocateIoApic(size_t num_apic);

    void InitializeIoApic(size_t idx, u8 id, u32 address, u32 gsi_base);

    void ApplyIoApicOverride(const acpi_madt_interrupt_source_override* override);

    void ApplyIoApicNmi(const acpi_madt_nmi_source* nmi_source);

    NODISCARD FORCE_INLINE_F IoApic& GetIoApic(const size_t idx)
    {
        byte* ptr = mem_ + idx * sizeof(IoApic);
        return *reinterpret_cast<IoApic*>(ptr);
    }

    NODISCARD IoApic& GetIoApicHandler(u32 gsi);

    // ------------------------------
    // Protected methods
    // ------------------------------

    protected:
    void InitializeDefaultIdt_();

    // ------------------------------
    // Class fields
    // ------------------------------

    Idt idt_{};

    TODO_WHEN_VMEM_WORKS
    alignas(IoApic) byte mem_[sizeof(IoApic) * 8]{};
    size_t num_apic_{};
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_INTERRUPTS_HPP_
