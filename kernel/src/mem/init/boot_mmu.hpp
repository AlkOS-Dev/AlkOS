// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MEM_INIT_BOOT_MMU_HPP_
#define KERNEL_SRC_MEM_INIT_BOOT_MMU_HPP_

#include <expected.hpp>
#include "hal/mmu.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"

namespace Mem::Boot
{

/**
 * @brief Boot MMU Context using BitmapPmm (before Buddy is ready).
 * Used for cleaning up bootloader tables.
 */
struct BootstrapMmuContext {
    Mem::BitmapPmm &pmm;
    Mem::PageMetaTable &pmt;

    std::expected<Mem::PPtr<void>, Mem::MemError> AllocateTable(uint8_t)
    {
        // Allocation is not supported during cleanup phase
        return std::unexpected(Mem::MemError::OutOfMemory);
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
        // Always assume empty/freeable during force cleanup
        return true;
    }
};

class BootMmuCleaner
{
    public:
    /**
     * @brief Clears the identity mappings (user/lower half) left by the bootloader.
     * This ensures the kernel starts with a clean user-space address range.
     */
    void CleanIdentityMappings(
        hal::Mmu &mmu, Mem::BitmapPmm &pmm, Mem::PageMetaTable &pmt, Mem::PPtr<void> root
    )
    {
        BootstrapMmuContext ctx{pmm, pmt};
        mmu.ClearUserMappings(ctx, root);
    }

    /**
     * @brief Reconstructs metadata (PageMetaTable) for the existing kernel page tables.
     * The bootloader creates the initial tables, but our memory manager needs
     * to know about them (type: PageTable, ref_count, etc.) to manage them later.
     */
    void ReconstructMetadata(hal::Mmu &mmu, Mem::PageMetaTable &pmt, Mem::PPtr<void> root)
    {
        mmu.VisitTables(root, [&](Mem::PPtr<void> table, uint8_t level, size_t entry_count) {
            auto &meta = pmt.GetPageMeta(table);
            meta.InitPageTable(level);

            // Set the reference count based on the number of present entries
            // This prevents the kernel from prematurely freeing these tables.
            // Note: We cast entry_count to u16, assuming it fits (max 512).
            Mem::PageMeta::AsPageTable(meta).ref_count = static_cast<u16>(entry_count);
        });
    }
};

}  // namespace Mem::Boot

#endif  // KERNEL_SRC_MEM_INIT_BOOT_MMU_HPP_
