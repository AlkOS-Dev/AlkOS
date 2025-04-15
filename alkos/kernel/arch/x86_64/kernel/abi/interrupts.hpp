#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_INTERRUPTS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_INTERRUPTS_HPP_

#include <interrupts/idt.hpp>

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

    void InitializeIoApic(u8 id, u32 address, u32 gsi_base);

    // ------------------------------
    // Protected methods
    // ------------------------------

    protected:
    void InitializeDefaultIdt_();

    // ------------------------------
    // Class fields
    // ------------------------------

    Idt idt_{};
    //
    // alignas(Core) byte mem_[sizeof(Core) * 128]{};
    // size_t num_cores_{};
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_INTERRUPTS_HPP_
