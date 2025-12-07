#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_

#include "hal/impl/mmu.hpp"
#include "mem/heap.hpp"

#include <mem/types.hpp>
#include <types.hpp>

namespace arch
{

using std::expected;
using std::unexpected;

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

template <size_t kLevel = 0>
void DestroyPageMapEntry(Mem::VPtr<PageMapEntry<kLevel>> pme)
{
    static_assert(kLevel > 0);
    static_assert(kLevel <= 4);
}

template <size_t kLevel>
expected<Mem::VPtr<PageMapEntry<kLevel>>, Mem::MemError> Mmu::WalkToEntry(
    Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr, bool create_if_missing
)
{
    static_assert(kLevel > 0);
    static_assert(kLevel <= 4);

    static constexpr u64 kDefFlags = kPresentBit | kWriteBit | kUserAccessibleBit;

    auto pmt_l4 =
        reinterpret_cast<Mem::VPtr<PageMapTable<4>>>(Mem::PhysToVirt(as->PageTableRoot()));

    auto pme_l4 = Mem::PhysToVirt(pmt_l4[PmeIdx<4>(vaddr)]);
    if (!pme_l4->IsPresent()) {
        if (!create_if_missing) {
            return unexpected(Mem::MemError::NotFound);
        }

        auto res = Mem::KMalloc<PageMapTable<3>>();
        if (!res) {
            return unexpected(res.error());
        }

        pme_l4->SetNextLevelTable(Mem::VirtToPhys(*res), kDefFlags);
    }
    if constexpr (kLevel == 4)
        return pme_l4;

    auto pmt_l3 = Mem::PhysToVirt(pme_l4->GetNextLevelTable());
    auto pme_l3 = Mem::PhysToVirt(pmt_l3[PmeIdx<3>(vaddr)]);
    if (!pme_l3->IsPresent()) {
        if (!create_if_missing) {
            return unexpected(Mem::MemError::NotFound);
        }

        auto res = Mem::KMalloc<PageMapTable<2>>();
        if (!res) {
            return unexpected(res.error());
        }

        pme_l3->SetNextLevelTable(Mem::VirtToPhys(*res), kDefFlags);
    }
    if constexpr (kLevel == 3)
        return pme_l3;

    auto pmt_l2 = Mem::PhysToVirt(pme_l3->GetNextLevelTable());
    auto pme_l2 = Mem::PhysToVirt(pmt_l2[PmeIdx<2>(vaddr)]);
    if (!pme_l2->IsPresent()) {
        if (!create_if_missing) {
            return unexpected(Mem::MemError::NotFound);
        }

        auto res = Mem::KMalloc<PageMapTable<1>>();
        if (!res) {
            return unexpected(res.error());
        }

        pme_l2->SetNextLevelTable(Mem::VirtToPhys(*res), kDefFlags);
    }
    if constexpr (kLevel == 2)
        return pme_l2;

    auto pmt_l1 = Mem::PhysToVirt(pme_l2->GetNextLevelTable());
    auto pme_l1 = Mem::PhysToVirt(pmt_l1[PmeIdx<1>(vaddr)]);
    if constexpr (kLevel == 1)
        return pme_l1;

    return unexpected(Mem::MemError::InvalidArgument);
}

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_
