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
        meta.InitPageTable(level);
        return reinterpret_cast<Mem::PPtr<void>>(ptr);
    }

    void FreeTable(Mem::PPtr<void> table, uint8_t level)
    {
        (void)level;
        auto &meta = pmt.GetPageMeta(table);
        meta.InitAllocated(0);
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
    void Unmap(Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr)
    {
        arch::Mmu::Unmap(ctx, root, vaddr);
        tlb_->InvalidatePage(vaddr);
    }

    template <MmuContext Context>
    void ClearUserMappings(Context &ctx, Mem::PPtr<void> root)
    {
        arch::Mmu::ClearUserMappings(ctx, root);
        tlb_->FlushAll();
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
    void UnmapRange(KernelMmuContext &ctx, Mem::PPtr<void> root, Mem::VPtr<void> start, size_t size)
    {
        using namespace Mem;

        auto s = PtrToUptr(AlignDown(start, kPageSizeBytes));
        auto e = AlignUp(PtrToUptr(start) + size, kPageSizeBytes);

        for (auto addr = s; addr < e; addr += kPageSizeBytes) {
            arch::Mmu::Unmap(ctx, root, UptrToPtr<void>(addr));
        }

        tlb_->InvalidateRange(start, size);
    }
};

}  // namespace hal

#endif  // KERNEL_SRC_HAL_MMU_HPP_
