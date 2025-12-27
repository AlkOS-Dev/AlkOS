#include "hal/impl/mmu.hpp"

#include <macros.hpp>
#include <modules/memory.hpp>

#include "mem/page_map.hpp"

using namespace arch;
using namespace Mem;

using std::expected;
using std::unexpected;

namespace
{

// Helper to get PageMeta from a physical pointer to a table/page
PageMeta &GetPageMeta(PPtr<void> phys_ptr)
{
    auto *aligned_phys = AlignDown(phys_ptr, hal::kPageSizeBytes);
    return MemoryModule::Get().GetPageMetaTable().GetPageMeta(aligned_phys);
}

// Helper to get PageMeta from a virtual pointer (e.g. inside a table)
PageMeta &GetPageMetaFromVirt(VPtr<void> virt_ptr)
{
    auto *phys_ptr = VirtToPhys(virt_ptr);
    return GetPageMeta(phys_ptr);
}

}  // namespace

u64 Mmu::ToArchFlags(PageFlags flags)
{
    u64 arch_flags = 0;
    if (flags.Present) {
        arch_flags |= kPresentBit;
    }
    if (flags.Writable) {
        arch_flags |= kWriteBit;
    }
    if (flags.UserAccessible) {
        arch_flags |= kUserAccessibleBit;
    }
    if (flags.WriteThrough) {
        arch_flags |= kWriteThroughCachingBit;
    }
    if (flags.CacheDisable) {
        arch_flags |= kDisableCacheBit;
    }
    if (flags.Global) {
        arch_flags |= kGlobalBit;
    }
    if (flags.NoExecute) {
        arch_flags |= kNoExecuteBit;
    }
    return arch_flags;
}

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

template <size_t kLevel>
expected<Mem::VPtr<PageMapEntry<kLevel>>, Mem::MemError> Mmu::WalkToEntry(
    Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr, bool create_if_missing
)
{
    static_assert(kLevel > 0);
    static_assert(kLevel <= 4);

    static constexpr u64 kDefFlags = kPresentBit | kWriteBit | kUserAccessibleBit;

    // PML4
    auto *pmt_l4 = reinterpret_cast<PageMapEntry<4> *>(Mem::PhysToVirt(as->PageTableRoot()));
    auto *pme_l4 = &pmt_l4[PmeIdx<4>(vaddr)];

    if (!pme_l4->IsPresent()) {
        if (!create_if_missing) {
            return unexpected(Mem::MemError::NotFound);
        }

        auto res = Mem::KMalloc<PageMapTable<3>>();
        if (!res) {
            return unexpected(res.error());
        }

        memset(*res, 0, sizeof(PageMapTable<3>));

        // Initialize Metadata for the new PDP table
        auto *table_phys  = Mem::VirtToPhys(*res);
        auto &new_meta    = GetPageMeta(table_phys);
        auto &parent_meta = GetPageMetaFromVirt(pmt_l4);

        new_meta.InitPageTable(3, &parent_meta);

        pme_l4->SetNextLevelTable(table_phys, kDefFlags);

        // Increment PML4 ref count
        PageMeta::AsPageTable(parent_meta).ref_count++;
    }
    if constexpr (kLevel == 4) {
        return pme_l4;
    }

    // PDP (L3)
    auto *pmt_l3 =
        reinterpret_cast<PageMapEntry<3> *>(Mem::PhysToVirt(pme_l4->GetNextLevelTable()));
    auto *pme_l3 = &pmt_l3[PmeIdx<3>(vaddr)];

    if (!pme_l3->IsPresent()) {
        if (!create_if_missing) {
            return unexpected(Mem::MemError::NotFound);
        }

        auto res = Mem::KMalloc<PageMapTable<2>>();
        if (!res) {
            return unexpected(res.error());
        }

        memset(*res, 0, sizeof(PageMapTable<2>));

        // Initialize Metadata for the new PD table
        auto *table_phys  = Mem::VirtToPhys(*res);
        auto &new_meta    = GetPageMeta(table_phys);
        auto &parent_meta = GetPageMetaFromVirt(pmt_l3);

        new_meta.InitPageTable(2, &parent_meta);

        pme_l3->SetNextLevelTable(table_phys, kDefFlags);

        // Increment PDP ref count
        PageMeta::AsPageTable(parent_meta).ref_count++;
    }
    if constexpr (kLevel == 3)
        return pme_l3;

    // PD (L2)
    auto *pmt_l2 =
        reinterpret_cast<PageMapEntry<2> *>(Mem::PhysToVirt(pme_l3->GetNextLevelTable()));
    auto *pme_l2 = &pmt_l2[PmeIdx<2>(vaddr)];

    if (!pme_l2->IsPresent()) {
        if (!create_if_missing) {
            return unexpected(Mem::MemError::NotFound);
        }

        auto res = Mem::KMalloc<PageMapTable<1>>();
        if (!res) {
            return unexpected(res.error());
        }

        memset(*res, 0, sizeof(PageMapTable<1>));

        // Initialize Metadata for the new PT table
        auto *table_phys  = Mem::VirtToPhys(*res);
        auto &new_meta    = GetPageMeta(table_phys);
        auto &parent_meta = GetPageMetaFromVirt(pmt_l2);

        new_meta.InitPageTable(1, &parent_meta);

        pme_l2->SetNextLevelTable(table_phys, kDefFlags);

        // Increment PD ref count
        PageMeta::AsPageTable(parent_meta).ref_count++;
    }
    if constexpr (kLevel == 2) {
        return pme_l2;
    }

    // PT (L1)
    auto *pmt_l1 =
        reinterpret_cast<PageMapEntry<1> *>(Mem::PhysToVirt(pme_l2->GetNextLevelTable()));
    auto *pme_l1 = &pmt_l1[PmeIdx<1>(vaddr)];

    if constexpr (kLevel == 1) {
        return pme_l1;
    }

    return unexpected(Mem::MemError::InvalidArgument);
}

expected<void, MemError> Mmu::Map(
    VPtr<AddressSpace> as, VPtr<void> vaddr, PPtr<void> paddr, PageFlags flags
)
{
    auto res = WalkToEntry<1>(as, vaddr, true);
    UNEXPECTED_RET_IF_ERR(res);

    auto *pme_l1 = *res;
    if (pme_l1->IsPresent()) {
        // Page is already mapped, unmap first
        return unexpected(MemError::InvalidArgument);
    }

    pme_l1->SetFrameAddress(paddr, ToArchFlags(flags));

    // pme_l1 points inside the L1 table, so use GetPageMetaFromVirt to get the table's meta
    auto &meta = GetPageMetaFromVirt(pme_l1);
    PageMeta::AsPageTable(meta).ref_count++;

    MemoryModule::Get().GetTlb().InvalidatePage(vaddr);

    return {};
}

expected<void, MemError> Mmu::UnMap(VPtr<AddressSpace> as, VPtr<void> vaddr)
{
    // 1. Find Leaf (L1 entry)
    auto res = WalkToEntry<1>(as, vaddr, false);
    if (!res) {
        return {};  // Already unmapped (table lookup failed)
    }

    auto *pme_l1 = *res;
    if (!pme_l1->IsPresent()) {
        return {};  // Entry not present
    }

    // 2. Clear Entry
    pme_l1->SetFrameAddress(0, 0);

    // 3. Update RefCount
    // pme_l1 points inside the L1 table. AlignDown gets the table start.
    auto *l1_table_virt = AlignDown(pme_l1, hal::kPageSizeBytes);
    auto &l1_meta       = GetPageMetaFromVirt(l1_table_virt);

    auto *curr_meta = &l1_meta;
    u8 curr_level   = 1;

    PageMeta::AsPageTable(*curr_meta).ref_count--;

    // 4. Cleanup loop
    while (PageMeta::AsPageTable(*curr_meta).ref_count == 0 && curr_level < 4) {
        auto *parent_meta = PageMeta::AsPageTable(*curr_meta).parent;
        if (parent_meta == nullptr) {
            break;
        }

        // We need to clear the entry in the parent that points to the current table.
        // Get parent table physical address from its metadata
        size_t parent_pfn = MemoryModule::Get().GetPageMetaTable().GetPageFrameNumber(parent_meta);
        auto *parent_table_virt =
            reinterpret_cast<u64 *>(Mem::PhysToVirt(PageFrameAddr(parent_pfn)));

        u64 idx = 0;
        if (curr_level == 1) {
            idx = PmeIdx<2>(vaddr);
        } else if (curr_level == 2) {
            idx = PmeIdx<3>(vaddr);
        } else if (curr_level == 3) {
            idx = PmeIdx<4>(vaddr);
        }

        // Clear entry in parent
        parent_table_virt[idx] = 0;

        // Free current table page
        size_t curr_pfn = MemoryModule::Get().GetPageMetaTable().GetPageFrameNumber(curr_meta);
        Mem::KFree(Mem::PhysToVirt(PageFrameAddr(curr_pfn)));

        // Move up to parent
        curr_meta = parent_meta;
        curr_level++;
        PageMeta::AsPageTable(*curr_meta).ref_count--;
    }

    MemoryModule::Get().GetTlb().InvalidatePage(vaddr);

    return {};
}

expected<PPtr<void>, MemError> Mmu::Translate(VPtr<AddressSpace> as, VPtr<void> vaddr)
{
    auto res = WalkToEntry<1>(as, vaddr, false);
    UNEXPECTED_RET_IF_ERR(res);

    auto pme_l1 = *res;
    if (!pme_l1->IsPresent()) {
        return unexpected(MemError::InvalidArgument);
    }

    return pme_l1->GetFrameAddress();
}
