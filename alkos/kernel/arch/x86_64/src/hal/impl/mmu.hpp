#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_

#include <hal/api/mmu.hpp>
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

    private:
    template <size_t kLevel = 0>
    PageMapEntry<kLevel> WalkPageEntries(Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr);

    template <size_t kLevel>
    u64 PmeIdx(Mem::VPtr<void> vaddr);
    u64 ToArchFlags(PageFlags flags);
};

}  // namespace arch

#include "hal/impl/mmu.tpp"

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_
