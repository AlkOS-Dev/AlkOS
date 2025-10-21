#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TLB_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TLB_HPP_

#include <hal/api/tlb.hpp>

#include <mem/virt/addr_space.hpp>
#include "cpu/control_registers.hpp"

namespace arch
{

class Tlb : public TlbAPI
{
    public:
    void FlushAll()
    {
        cpu::Cr3 cr3 = cpu::GetCR<cpu::Cr3>();
        cpu::SetCR(cr3);
    }

    void InvalidatePage(Mem::VPtr<void> vaddr)
    {
        asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
    }

    void SwitchAddrSpace(Mem::VPtr<Mem::AddressSpace> as)
    {
        cpu::Cr3 cr3{};
        cr3.PageMapLevel4Address = Mem::PtrToUptr(as->PageTableRoot()) >> 12;
        cpu::SetCR(cr3);
    }

    private:
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TLB_HPP_
