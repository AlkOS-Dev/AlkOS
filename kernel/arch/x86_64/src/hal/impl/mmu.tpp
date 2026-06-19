// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_

#include "hal/impl/mmu.hpp"
#include "internal/macros.hpp"
#include "mem/page_map.hpp"

#include <string.h>

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

template <MmuContext Context>
expected<void, Mem::MemError> Mmu::Map(
    Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr, Mem::PPtr<void> paddr,
    PageFlags flags
)
{
    static constexpr u64 kDefTableFlags = kPresentBit | kWriteBit | kUserAccessibleBit;

    // Level 4
    auto *pml4  = reinterpret_cast<PageMapTable<4> *>(Mem::PhysToVirt(root));
    auto &pml4e = (*pml4)[PmeIdx<4>(vaddr)];

    if (!pml4e.IsPresent()) {
        auto res = ctx.AllocateTable(3);
        RET_UNEXPECTED_IF_ERR(res);
        pml4e.SetNextLevelTable(reinterpret_cast<PageMapTable<3> *>(*res), kDefTableFlags);
        ctx.IncreaseUsage(root);
    }

    // Level 3
    auto *pdpt  = reinterpret_cast<PageMapTable<3> *>(Mem::PhysToVirt(pml4e.GetNextLevelTable()));
    auto &pdpte = (*pdpt)[PmeIdx<3>(vaddr)];

    if (!pdpte.IsPresent()) {
        auto res = ctx.AllocateTable(2);
        RET_UNEXPECTED_IF_ERR(res);
        pdpte.SetNextLevelTable(reinterpret_cast<PageMapTable<2> *>(*res), kDefTableFlags);
        ctx.IncreaseUsage(reinterpret_cast<Mem::PPtr<void>>(pml4e.GetNextLevelTable()));
    }

    // Level 2
    auto *pd  = reinterpret_cast<PageMapTable<2> *>(Mem::PhysToVirt(pdpte.GetNextLevelTable()));
    auto &pde = (*pd)[PmeIdx<2>(vaddr)];

    if (!pde.IsPresent()) {
        auto res = ctx.AllocateTable(1);
        RET_UNEXPECTED_IF_ERR(res);
        pde.SetNextLevelTable(reinterpret_cast<PageMapTable<1> *>(*res), kDefTableFlags);
        ctx.IncreaseUsage(reinterpret_cast<Mem::PPtr<void>>(pdpte.GetNextLevelTable()));
    }

    // Level 1
    auto *pt  = reinterpret_cast<PageMapTable<1> *>(Mem::PhysToVirt(pde.GetNextLevelTable()));
    auto &pte = (*pt)[PmeIdx<1>(vaddr)];

    RET_UNEXPECTED_IF(pte.IsPresent(), Mem::MemError::InvalidArgument);

    pte.SetFrameAddress(paddr, ToArchFlags(flags));
    ctx.IncreaseUsage(reinterpret_cast<Mem::PPtr<void>>(pde.GetNextLevelTable()));

    return {};
}

template <MmuContext Context>
void Mmu::Unmap(Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr)
{
    // Track path for cleanup
    struct PathEntry {
        Mem::PPtr<void> table_phys;
        u64 index;
    };
    PathEntry path[4];  // Indices: 0=PML4, 1=PDPT, 2=PD, 3=PT

    // Walk down
    auto *pml4  = reinterpret_cast<PageMapTable<4> *>(Mem::PhysToVirt(root));
    path[0]     = {root, PmeIdx<4>(vaddr)};
    auto &pml4e = (*pml4)[path[0].index];
    if (!pml4e.IsPresent())
        return;

    path[1]     = {reinterpret_cast<Mem::PPtr<void>>(pml4e.GetNextLevelTable()), PmeIdx<3>(vaddr)};
    auto *pdpt  = reinterpret_cast<PageMapTable<3> *>(Mem::PhysToVirt(path[1].table_phys));
    auto &pdpte = (*pdpt)[path[1].index];
    if (!pdpte.IsPresent())
        return;

    path[2]   = {reinterpret_cast<Mem::PPtr<void>>(pdpte.GetNextLevelTable()), PmeIdx<2>(vaddr)};
    auto *pd  = reinterpret_cast<PageMapTable<2> *>(Mem::PhysToVirt(path[2].table_phys));
    auto &pde = (*pd)[path[2].index];
    if (!pde.IsPresent())
        return;

    path[3]   = {reinterpret_cast<Mem::PPtr<void>>(pde.GetNextLevelTable()), PmeIdx<1>(vaddr)};
    auto *pt  = reinterpret_cast<PageMapTable<1> *>(Mem::PhysToVirt(path[3].table_phys));
    auto &pte = (*pt)[path[3].index];
    if (!pte.IsPresent())
        return;

    // Clear leaf
    pte.Clear();

    // Bubble up cleanup
    // Start checking from the leaf table (level 1, path[3]) up to PDPT (level 3, path[1])
    // If a table becomes empty, remove it from its parent.

    // Check PT (Level 1)
    if (ctx.DecreaseUsage(path[3].table_phys)) {
        // PT is empty, free it and remove from PD
        ctx.FreeTable(path[3].table_phys, 1);

        auto *parent_pd  = reinterpret_cast<PageMapTable<2> *>(Mem::PhysToVirt(path[2].table_phys));
        auto &parent_pde = (*parent_pd)[path[2].index];
        parent_pde.Clear();  // Clear PD entry

        // Check PD (Level 2)
        if (ctx.DecreaseUsage(path[2].table_phys)) {
            // PD is empty, free it and remove from PDPT
            ctx.FreeTable(path[2].table_phys, 2);

            auto *parent_pdpt =
                reinterpret_cast<PageMapTable<3> *>(Mem::PhysToVirt(path[1].table_phys));
            auto &parent_pdpte = (*parent_pdpt)[path[1].index];
            parent_pdpte.Clear();

            // Check PDPT (Level 3)
            if (ctx.DecreaseUsage(path[1].table_phys)) {
                // PDPT is empty, free it and remove from PML4
                ctx.FreeTable(path[1].table_phys, 3);

                auto *parent_pml4 =
                    reinterpret_cast<PageMapTable<4> *>(Mem::PhysToVirt(path[0].table_phys));
                auto &parent_pml4e = (*parent_pml4)[path[0].index];
                parent_pml4e.Clear();

                // Decrease usage of PML4
                ctx.DecreaseUsage(path[0].table_phys);
            }
        }
    } else {
        // Leaf table not empty, nothing more to do
    }
}

template <MmuContext Context>
void Mmu::ClearUserMappings(Context &ctx, Mem::PPtr<void> root)
{
    // On x86_64, the lower half of the address space corresponds to
    // the first 256 entries of PML4.
    constexpr size_t kLowerHalfEntries = 256;

    auto *pml4 = reinterpret_cast<PageMapTable<4> *>(Mem::PhysToVirt(root));

    for (size_t i = 0; i < kLowerHalfEntries; ++i) {
        auto &entry = (*pml4)[i];
        if (entry.IsPresent()) {
            // Recursively destroy the sub-tables
            DestroyTable(ctx, reinterpret_cast<Mem::PPtr<void>>(entry.GetNextLevelTable()), 3);
            entry.Clear();
        }
    }
}

template <MmuContext Context>
expected<Mem::PPtr<void>, Mem::MemError> Mmu::Translate(
    Context &, Mem::PPtr<void> root, Mem::VPtr<void> vaddr
)
{
    auto *pml4  = reinterpret_cast<PageMapTable<4> *>(Mem::PhysToVirt(root));
    auto &pml4e = (*pml4)[PmeIdx<4>(vaddr)];
    RET_UNEXPECTED_IF(!pml4e.IsPresent(), Mem::MemError::NotFound);

    auto *pdpt  = reinterpret_cast<PageMapTable<3> *>(Mem::PhysToVirt(pml4e.GetNextLevelTable()));
    auto &pdpte = (*pdpt)[PmeIdx<3>(vaddr)];
    RET_UNEXPECTED_IF(!pdpte.IsPresent(), Mem::MemError::NotFound);
    if (pdpte.IsHuge())
        return reinterpret_cast<const PageMapEntry<3, kHugePage> &>(pdpte).GetFrameAddress() +
               (Mem::PtrToUptr(vaddr) & kBitMaskRight<u64, 30>);

    auto *pd  = reinterpret_cast<PageMapTable<2> *>(Mem::PhysToVirt(pdpte.GetNextLevelTable()));
    auto &pde = (*pd)[PmeIdx<2>(vaddr)];
    RET_UNEXPECTED_IF(!pde.IsPresent(), Mem::MemError::NotFound);
    if (pde.IsHuge())
        return reinterpret_cast<const PageMapEntry<2, kHugePage> &>(pde).GetFrameAddress() +
               (Mem::PtrToUptr(vaddr) & kBitMaskRight<u64, 21>);

    auto *pt  = reinterpret_cast<PageMapTable<1> *>(Mem::PhysToVirt(pde.GetNextLevelTable()));
    auto &pte = (*pt)[PmeIdx<1>(vaddr)];
    RET_UNEXPECTED_IF(!pte.IsPresent(), Mem::MemError::NotFound);

    return pte.GetFrameAddress() + (Mem::PtrToUptr(vaddr) & kBitMaskRight<u64, 12>);
}

template <TableVisitor Visitor>
void Mmu::VisitTables(Mem::PPtr<void> root, Visitor visitor)
{
    // Visitor signature: void(Mem::PPtr<void> table, uint8_t level, size_t entry_count)

    // We traverse recursively
    auto RecVisit = [&]<size_t Level>(this auto &&self, Mem::PPtr<void> table_phys) -> void {
        auto *table = reinterpret_cast<PageMapTable<Level> *>(Mem::PhysToVirt(table_phys));

        // First pass: count entries
        size_t count = 0;
        for (const auto &entry : *table) {
            if (entry.IsPresent()) {
                count++;
            }
        }

        // Invoke visitor with stats
        visitor(table_phys, Level, count);

        if constexpr (Level > 1) {
            for (const auto &entry : *table) {
                if (entry.IsPresent() && !entry.IsHuge()) {
                    self.template operator()<Level - 1>(
                        reinterpret_cast<Mem::PPtr<void>>(entry.GetNextLevelTable())
                    );
                }
            }
        }
    };

    RecVisit.template operator()<4>(root);
}

template <MmuContext Context>
void Mmu::DestroyTable(Context &ctx, Mem::PPtr<void> table_phys, u8 level)
{
    // Recursive destruction

    auto RecDestroy = [&]<size_t Level>(this auto &&self, Mem::PPtr<void> curr_table) -> void {
        auto *table = reinterpret_cast<PageMapTable<Level> *>(Mem::PhysToVirt(curr_table));

        if constexpr (Level > 1) {
            for (auto &entry : *table) {
                if (entry.IsPresent() && !entry.IsHuge()) {
                    self.template operator()<Level - 1>(
                        reinterpret_cast<Mem::PPtr<void>>(entry.GetNextLevelTable())
                    );
                }
                entry.Clear();
            }
        } else {
            // Level 1: Just clear entries
            for (auto &entry : *table) {
                entry.Clear();
            }
        }

        ctx.FreeTable(curr_table, Level);
    };

    switch (level) {
        case 4:
            RecDestroy.template operator()<4>(table_phys);
            break;
        case 3:
            RecDestroy.template operator()<3>(table_phys);
            break;
        case 2:
            RecDestroy.template operator()<2>(table_phys);
            break;
        case 1:
            RecDestroy.template operator()<1>(table_phys);
            break;
    }
}

}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_TPP_
