#ifndef KERNEL_SRC_MEM_MMU_CONTEXTS_HPP_
#define KERNEL_SRC_MEM_MMU_CONTEXTS_HPP_

#include <expected.hpp>
#include <hal/api/mmu.hpp>
#include <macros.hpp>

#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/types.hpp"

namespace Mem
{

/**
 * @brief Default MMU Context using the full kernel memory subsystem.
 */
struct KernelMmuContext {
    Mem::BuddyPmm &pmm;
    Mem::PageMetaTable &pmt;

    std::expected<Mem::PPtr<void>, Mem::MemError> AllocateTable(uint8_t level)
    {
        auto res = pmm.Alloc({.order = 0});
        if (!res)
            return std::unexpected(res.error());
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

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_MMU_CONTEXTS_HPP_
