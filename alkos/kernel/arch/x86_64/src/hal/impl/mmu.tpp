#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_

#include "hal/impl/mmu.hpp"

#include <extensions/types.hpp>
#include <mem/types.hpp>

namespace arch
{

template <size_t kLevel>
u64 Mmu::PmeIdx(Mem::VPtr<void> vaddr)
{
    static_assert(kLevel > 0);
    static_assert(kLevel <= 4);

    static constexpr u64 kDefaultOffset     = 12;
    static constexpr u64 kBitOffsetPerLevel = 9;

    static constexpr u32 kIndexMask = kBitMaskRight<u64, 9>;
    uptr addr                       = Mem::PtrToUptr(vaddr);
    return (addr >> (kDefaultOffset + (kLevel - 1) * kBitOffsetPerLevel)) & kIndexMask;
}

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_
