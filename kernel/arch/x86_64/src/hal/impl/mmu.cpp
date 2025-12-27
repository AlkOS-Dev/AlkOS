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
// Used by Map/UnMap where MemoryModule is fully initialized
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

// Helper to get PageMeta using explicit PageMetaTable (safe during initialization)
PageMeta &GetPageMeta(PPtr<void> phys_ptr, PageMetaTable &pmt)
{
    auto *aligned_phys = AlignDown(phys_ptr, hal::kPageSizeBytes);
    return pmt.GetPageMeta(aligned_phys);
}

// Helper to recursively free page table frames
template <size_t kLevel>
void FreeTableRecursive(PPtr<void> table_phys, PageMetaTable &pmt, BitmapPmm &pmm)
{
    auto *table_virt = reinterpret_cast<PageMapTable<kLevel> *>(PhysToVirt(table_phys));

    for (auto &entry : *table_virt) {
        if (entry.IsPresent()) {
            bool is_huge = false;
            if constexpr (kLevel > 1) {
                is_huge = entry.page_size;
            }

            if constexpr (kLevel > 1) {
                if (!is_huge) {
                    PPtr<void> child_phys = reinterpret_cast<PPtr<void>>(entry.GetNextLevelTable());
                    FreeTableRecursive<kLevel - 1>(child_phys, pmt, pmm);
                }
            }
            // If it is a leaf (Level 1, or Huge Level 2/3), we do NOTHING.
            // We are only destroying the mapping structure, not the memory it points to.
        }
    }

    // Now free the table frame itself
    pmm.Free(reinterpret_cast<PPtr<Page>>(table_phys));

    // Invalidate metadata
    auto &meta = GetPageMeta(table_phys, pmt);
    meta.type  = PageMetaType::Dummy;
}

template <size_t kLevel>
void ReconstructRecursive(PPtr<void> table_phys, PageMeta *parent, PageMetaTable &pmt)
{
    // Initialize metadata for the current table frame
    auto &meta = GetPageMeta(table_phys, pmt);
    meta.InitPageTable(kLevel, parent);

    auto *table_virt = reinterpret_cast<PageMapTable<kLevel> *>(PhysToVirt(table_phys));

    u16 ref_count = 0;
    for (const auto &entry : *table_virt) {
        if (entry.IsPresent()) {
            ref_count++;
            if constexpr (kLevel > 1) {
                if (!entry.page_size) {
                    PPtr<void> child_phys = reinterpret_cast<PPtr<void>>(entry.GetNextLevelTable());
                    ReconstructRecursive<kLevel - 1>(child_phys, &meta, pmt);
                }
                // If page_size is set, then it's a huge page
            }
        }
    }
    meta.data.page_table.ref_count = ref_count;
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

    // Helper lambda to process each level
    auto ProcessLevel = [&]<size_t L>(
                            Mem::VPtr<PageMapEntry<L>> pme, bool create
                        ) -> expected<Mem::VPtr<PageMapEntry<L - 1>>, Mem::MemError> {
        if (pme->IsPresent()) {
            return reinterpret_cast<PageMapEntry<L - 1> *>(
                Mem::PhysToVirt(pme->GetNextLevelTable())
            );
        }

        if (!create) {
            return unexpected(Mem::MemError::NotFound);
        }

        auto res = Mem::KMalloc<PageMapTable<L - 1>>();
        if (!res) {
            return unexpected(res.error());
        }

        memset(*res, 0, sizeof(PageMapTable<L - 1>));

        // Initialize Metadata for the new table
        auto *table_phys = Mem::VirtToPhys(*res);
        auto &new_meta   = GetPageMeta(table_phys);
        // pme points inside the parent table
        auto &parent_meta = GetPageMetaFromVirt(pme);

        new_meta.InitPageTable(L - 1, &parent_meta);
        pme->SetNextLevelTable(table_phys, kDefFlags);

        // Increment parent ref count
        PageMeta::AsPageTable(parent_meta).ref_count++;

        return reinterpret_cast<PageMapEntry<L - 1> *>(*res);
    };

    // PML4
    auto *pmt_l4 = reinterpret_cast<PageMapEntry<4> *>(Mem::PhysToVirt(as->PageTableRoot()));
    auto *pme_l4 = &pmt_l4[PmeIdx<4>(vaddr)];

    if constexpr (kLevel == 4) {
        return pme_l4;
    }

    // PDP (L3)
    auto res_l3 = ProcessLevel.template operator()<4>(pme_l4, create_if_missing);
    if (!res_l3) {
        return unexpected(res_l3.error());
    }
    auto *pme_l3 = &(*res_l3)[PmeIdx<3>(vaddr)];

    if constexpr (kLevel == 3) {
        return pme_l3;
    }

    // PD (L2)
    auto res_l2 = ProcessLevel.template operator()<3>(pme_l3, create_if_missing);
    if (!res_l2) {
        return unexpected(res_l2.error());
    }
    auto *pme_l2 = &(*res_l2)[PmeIdx<2>(vaddr)];

    if constexpr (kLevel == 2) {
        return pme_l2;
    }

    // PT (L1)
    auto res_l1 = ProcessLevel.template operator()<2>(pme_l2, create_if_missing);
    if (!res_l1) {
        return unexpected(res_l1.error());
    }
    auto *pme_l1 = &(*res_l1)[PmeIdx<1>(vaddr)];

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
        // Page is already mapped
        return unexpected(MemError::InvalidArgument);
    }

    pme_l1->SetFrameAddress(paddr, ToArchFlags(flags));

    // Increment Page Table (L1) ref count
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
        return {};  // Already unmapped
    }

    auto *pme_l1 = *res;
    if (!pme_l1->IsPresent()) {
        return {};  // Entry not present
    }

    // 2. Clear Entry
    pme_l1->SetFrameAddress(0, 0);

    // 3. Update RefCount
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

    auto *pme_l1 = *res;
    if (!pme_l1->IsPresent()) {
        return unexpected(MemError::InvalidArgument);
    }

    return pme_l1->GetFrameAddress();
}

void Mmu::ReconstructAddressSpace(Mem::PPtr<void> root_page_table, Mem::PageMetaTable &pmt)
{
    ReconstructRecursive<4>(root_page_table, nullptr, pmt);
}

void Mmu::UnmapLowerHalf(
    Mem::PPtr<void> root_page_table, Mem::PageMetaTable &pmt, Mem::BitmapPmm &pmm, hal::Tlb &tlb
)
{
    auto *pml4_virt = reinterpret_cast<PageMapTable<4> *>(PhysToVirt(root_page_table));
    auto &pml4_meta = GetPageMeta(root_page_table, pmt);

    constexpr size_t kLowerHalfLimit = 256;
    for (size_t i = 0; i < kLowerHalfLimit; ++i) {
        auto &entry = (*pml4_virt)[i];
        if (entry.IsPresent()) {
            // Free the table the entry points to
            PPtr<void> pdpt_phys = reinterpret_cast<PPtr<void>>(entry.GetNextLevelTable());
            FreeTableRecursive<3>(pdpt_phys, pmt, pmm);

            // Free the entry
            *reinterpret_cast<u64 *>(&entry) = 0;
        }
    }

    // Flush TLB to ensure CPU doesn't retain old mappings
    tlb.FlushAll();
}
