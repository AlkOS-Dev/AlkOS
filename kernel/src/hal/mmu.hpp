#ifndef KERNEL_SRC_HAL_MMU_HPP_
#define KERNEL_SRC_HAL_MMU_HPP_

#include <hal/impl/mmu.hpp>
#include <macros.hpp>

#include "hal/constants.hpp"
#include "hal/tlb.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/phys/mngr/buddy.hpp"

namespace hal
{

using arch::MmuContext;

using std::expected;
using std::unexpected;

using PageFlags = arch::PageFlags;

/**
 * @brief Default MMU Context using the full kernel memory subsystem.
 */
struct KernelMmuContext {
    Mem::BuddyPmm &pmm;
    Mem::PageMetaTable &pmt;

    expected<Mem::PPtr<void>, Mem::MemError> AllocateTable(uint8_t level)
    {
        auto res = pmm.Alloc({.order = 0});
        if (!res)
            return unexpected(res.error());
        auto ptr = *res;
        memset(Mem::PhysToVirt(ptr), 0, hal::kPageSizeBytes);

        auto &meta = pmt.GetPageMeta(ptr);
        // Parent tracking is not strictly enforced by this context during alloc,
        // but we init the metadata type.
        meta.InitPageTable(level);
        return reinterpret_cast<Mem::PPtr<void>>(ptr);
    }

    void FreeTable(Mem::PPtr<void> table, uint8_t level)
    {
        (void)level;
        auto &meta = pmt.GetPageMeta(table);
        meta.InitAllocated(0);  // Reset type to generic allocated before freeing
        pmm.Free(reinterpret_cast<Mem::PPtr<Mem::Page>>(table));
    }

    void IncreaseUsage(Mem::PPtr<void> table)
    {
        auto &meta = pmt.GetPageMeta(table);
        Mem::PageMeta::AsPageTable(meta).ref_count++;
    }

    bool DecreaseUsage(Mem::PPtr<void> table)
    {
        auto &meta = pmt.GetPageMeta(table);
        auto &pt   = Mem::PageMeta::AsPageTable(meta);
        if (pt.ref_count > 0)
            pt.ref_count--;
        return pt.ref_count == 0;
    }
};

/**
 * @brief Boot MMU Context using BitmapPmm (before Buddy is ready).
 * Used for cleaning up bootloader tables.
 */
struct BootstrapMmuContext {
    Mem::BitmapPmm &pmm;
    Mem::PageMetaTable &pmt;

    expected<Mem::PPtr<void>, Mem::MemError> AllocateTable(uint8_t)
    {
        // Not implemented/Needed for cleanup
        return unexpected(Mem::MemError::OutOfMemory);
    }

    void FreeTable(Mem::PPtr<void> table, uint8_t level)
    {
        (void)level;
        auto &meta = pmt.GetPageMeta(table);
        meta.InitAllocated(0);
        pmm.Free(reinterpret_cast<Mem::PPtr<Mem::Page>>(table));
    }

    void IncreaseUsage(Mem::PPtr<void>) {}
    bool DecreaseUsage(Mem::PPtr<void>)
    {
        return true;
    }  // Always assume empty/freeable during force cleanup
};

class Mmu : public arch::Mmu
{
    Tlb *tlb_;

    public:
    void Init(Tlb &tlb) { tlb_ = &tlb; }

    /// The range is treated as half-open: [start, start + size)
    expected<void, Mem::MemError> MapRange(
        KernelMmuContext &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr_start,
        Mem::PPtr<void> paddr_start, size_t size, PageFlags flags
    )
    {
        using namespace Mem;

        auto vaddr = PtrToUptr(AlignDown(vaddr_start, kPageSizeBytes));
        auto paddr = PtrToUptr(AlignDown(paddr_start, kPageSizeBytes));
        auto end   = AlignUp(PtrToUptr(vaddr_start) + size, kPageSizeBytes);

        for (auto v = vaddr, p = paddr; v < end; v += kPageSizeBytes, p += kPageSizeBytes) {
            auto map_res = Map(ctx, root, UptrToPtr<void>(v), UptrToPtr<void>(p), flags);
            RET_UNEXPECTED_IF_ERR(map_res);
        }
        return {};
    }

    template <MmuContext Context>
    expected<void, Mem::MemError> Unmap(Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr)
    {
        auto res = arch::Mmu::Unmap(ctx, root, vaddr);
        if (res)
            tlb_->InvalidatePage(vaddr);
        return res;
    }

    expected<void, Mem::MemError> SetPageFlags(
        Mem::PPtr<void> root, Mem::VPtr<void> vaddr, PageFlags flags
    )
    {
        auto res = arch::Mmu::SetPageFlags(root, vaddr, flags);
        if (res)
            tlb_->InvalidatePage(vaddr);
        return {};
    }

    /// The range is treated as half-open: [start, start + size)
    expected<void, Mem::MemError> UnmapRange(
        KernelMmuContext &ctx, Mem::PPtr<void> root, Mem::VPtr<void> start, size_t size
    )
    {
        using namespace Mem;

        auto s = PtrToUptr(AlignDown(start, kPageSizeBytes));
        auto e = AlignUp(PtrToUptr(start) + size, kPageSizeBytes);

        for (auto addr = s; addr < e; addr += kPageSizeBytes) {
            auto unmap_res = arch::Mmu::Unmap(ctx, root, UptrToPtr<void>(addr));
        }

        tlb_->InvalidateRange(start, size);
        return {};
    }

    /// Reconstructs metadata for an existing page table tree
    void ReconstructAddressSpace(Mem::PPtr<void> root, Mem::PageMetaTable &pmt)
    {
        // Counts refs for a table by inspecting its entries
        // Note: this assumes we are iterating a valid tree
        // The Visitor in arch::Mmu doesn't provide entry counts, so we do it manually or
        // rely on the fact that we can just set type for now.
        // To strictly restore ref_counts, we need to count bits.

        VisitTables(root, [&](Mem::PPtr<void> table, uint8_t level) {
            auto &meta = pmt.GetPageMeta(table);
            meta.InitPageTable(level);

            // Count entries for ref_count
            if (level > 1) {
                u16 count = 0;
                // Hack: we cast to generic array to count present bits
                // This assumes standard x86 layout which HAL technically shouldn't know,
                // but Mmu inherits from arch::Mmu so it can access Arch defs if included.
                // We can't access PageMapTable<L> easily here without template.
                // Let's rely on a simplified assumption or add a helper in Arch.
                // For now, we set ref_count to 1 to prevent accidental freeing
                // until we implement proper counting or let the system run.
                // Ideally, we would iterate the table.
                auto *virt = reinterpret_cast<u64 *>(Mem::PhysToVirt(table));
                for (int i = 0; i < 512; ++i) {
                    if (virt[i] & 1)
                        count++;
                }
                Mem::PageMeta::AsPageTable(meta).ref_count = count;
            } else {
                // Level 1 (PT)
                u16 count  = 0;
                auto *virt = reinterpret_cast<u64 *>(Mem::PhysToVirt(table));
                for (int i = 0; i < 512; ++i) {
                    if (virt[i] & 1)
                        count++;
                }
                Mem::PageMeta::AsPageTable(meta).ref_count = count;
            }
        });
    }

    void UnmapLowerHalf(Mem::PPtr<void> root, Mem::PageMetaTable &pmt, Mem::BitmapPmm &pmm)
    {
        BootstrapMmuContext ctx{pmm, pmt};

        // Manual iteration of PML4 lower half
        auto *pml4                         = reinterpret_cast<u64 *>(Mem::PhysToVirt(root));
        constexpr size_t kLowerHalfEntries = 256;

        for (size_t i = 0; i < kLowerHalfEntries; ++i) {
            if (pml4[i] & 1) {  // Present
                Mem::PPtr<void> child =
                    reinterpret_cast<Mem::PPtr<void>>(pml4[i] & 0x000FFFFFFFFFF000);  // Mask flags
                DestroyTable(ctx, child, 3);  // Child is Level 3 (PDPT)
                pml4[i] = 0;
            }
        }

        // TLB flush should be handled by caller or here
        SwitchRoot(root);  // Reload CR3 flushes TLB
    }
};

}  // namespace hal

#endif  // KERNEL_SRC_HAL_MMU_HPP_
