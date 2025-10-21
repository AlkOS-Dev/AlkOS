#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_

#include <hal/api/mmu.hpp>
#include <mem/virt/addr_space.hpp>
#include "cpu/control_registers.hpp"
#include "mem/page_map.hpp"

namespace arch
{

class Mmu : public MmuAPI
{
    public:
    Expected<void, Mem::MemError> Map(
        Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr, Mem::PPtr<void> paddr, PageFlags
    );

    Expected<void, Mem::MemError> UnMap(Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr);

    Expected<Mem::PPtr<void>, Mem::MemError> Translate(
        Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr
    );

    void SwitchAddrSpace(Mem::VPtr<Mem::AddressSpace> as)
    {
        cpu::Cr3 cr3{};
        cr3.PageMapLevel4Address = Mem::PtrToUptr(as->PageTableRoot()) >> 12;
        cpu::SetCR(cr3);
    }

    private:
    template <size_t kLevel = 0>
    Expected<Mem::VPtr<PageMapEntry<kLevel>>, Mem::MemError> WalkToEntry(
        Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr, bool create_if_missing = false
    );

    template <size_t kLevel>
    u64 PmeIdx(Mem::VPtr<void> vaddr);
    u64 ToArchFlags(PageFlags flags);
};

}  // namespace arch

#include "hal/impl/mmu.tpp"

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_
