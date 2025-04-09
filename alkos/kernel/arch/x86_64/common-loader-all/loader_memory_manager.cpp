#include "loader_memory_manager.hpp"
#include <assert.h>
#include <memory.h>
#include <extensions/algorithm.hpp>
#include <extensions/debug.hpp>
#include <extensions/internal/intervals.hpp>

// Note: The alignment here is a strict requirement for the PML tables and if the
// initial object is not aligned, the PML tables will not be aligned either.
alignas(4096) byte kLoaderPreAllocatedMemory[sizeof(LoaderMemoryManager)];

LoaderMemoryManager::LoaderMemoryManager()
{
    TRACE_INFO("LoaderMemoryManager::LoaderMemoryManager()");

    num_pml_tables_stored_ = 1;  ///< The first PML table is the PML4 table
    memset(descending_sorted_mmap_entries, 0, sizeof(descending_sorted_mmap_entries));
    memset(buffer_, 0, sizeof(u64) * kNumEntriesPerPml * kMaxPmlTablesToStore);

    // Should get compiled away in release mode
    for (u32 i = 0; i < kMaxPmlTablesToStore; i++) {
        auto *pml_table = reinterpret_cast<u64 *>(&buffer_[i]);
        for (u32 j = 0; j < kNumEntriesPerPml; j++) {
            ASSERT_ZERO(pml_table[j]);
        }
    }
}
LoaderMemoryManager::PML4_t *LoaderMemoryManager::GetPml4Table() { return &buffer_[kPml4Index]; }
void LoaderMemoryManager::AddFreeMemoryRegion(u64 start_addr, u64 end_addr)
{
    R_ASSERT_LT(num_free_memory_regions_, kMaxMemoryMapEntries);
    FreeMemoryRegion_t &mmap_entry_ref = descending_sorted_mmap_entries[num_free_memory_regions_++];
    mmap_entry_ref.addr                = start_addr;
    mmap_entry_ref.length              = end_addr - start_addr;
    available_memory_bytes_ += mmap_entry_ref.length;

    // Sort the memory map entries in descending order
    u32 i = num_free_memory_regions_ - 1;
    while (i > 0 &&
           descending_sorted_mmap_entries[i].addr > descending_sorted_mmap_entries[i - 1].addr) {
        byte temp[sizeof(FreeMemoryRegion_t)];
        memcpy(&temp, &descending_sorted_mmap_entries[i], sizeof(FreeMemoryRegion_t));
        memcpy(
            &descending_sorted_mmap_entries[i], &descending_sorted_mmap_entries[i - 1],
            sizeof(FreeMemoryRegion_t)
        );
        memcpy(&descending_sorted_mmap_entries[i - 1], &temp, sizeof(FreeMemoryRegion_t));
        i--;
    }
}
void LoaderMemoryManager::MarkMemoryAreaNotFree(u64 start_addr, u64 end_addr)
{
    bool found_intersection = true;
    while (found_intersection) {
        found_intersection = false;
        for (u32 i = 0; i < num_free_memory_regions_; i++) {
            constexpr i64 kNoOffset          = 0;
            constexpr u64 kEmptyRegionLength = 0;

            const i64 free_start = static_cast<i64>(descending_sorted_mmap_entries[i].addr);
            const i64 free_end   = static_cast<i64>(
                descending_sorted_mmap_entries[i].addr + descending_sorted_mmap_entries[i].length
            );
            const i64 region_start = static_cast<i64>(start_addr);
            const i64 region_end   = static_cast<i64>(end_addr);

            if (internal::DoOpenIntervalsOverlap(free_start, free_end, region_start, region_end)) {
                TRACE_INFO(
                    "Found intersection of 0x%llX-0x%llX with 0x%llX-0x%llX", region_start,
                    region_end, free_start, free_end
                );
                found_intersection = true;
                // Delete the not free part of the memory region

                // Case 0: The memory region completely encompasses the free memory region
                if (region_start <= free_start && region_end >= free_end) {
                    available_memory_bytes_ -= descending_sorted_mmap_entries[i].length;
                    descending_sorted_mmap_entries[i].length = kEmptyRegionLength;
                }

                // Case 1: The left part of the memory region is not free
                if (start_addr < descending_sorted_mmap_entries[i].addr) {
                    const u64 original_addr                = descending_sorted_mmap_entries[i].addr;
                    descending_sorted_mmap_entries[i].addr = end_addr;
                    const u64 lost_memory                  = end_addr - original_addr;
                    R_ASSERT_GE(descending_sorted_mmap_entries[i].length, lost_memory);
                    descending_sorted_mmap_entries[i].length -= lost_memory;
                    available_memory_bytes_ -= lost_memory;
                }
                // Case 2: The right part of the memory region is not free
                else if (end_addr > descending_sorted_mmap_entries[i].addr) {
                    const u64 original_addr = descending_sorted_mmap_entries[i].addr;
                    const u64 lost_memory =
                        (original_addr + descending_sorted_mmap_entries[i].length) - start_addr;
                    R_ASSERT_GE(descending_sorted_mmap_entries[i].length, lost_memory);
                    descending_sorted_mmap_entries[i].length = start_addr - original_addr;
                    available_memory_bytes_ -= lost_memory;
                }

                // Case 3: The memory region is completely inside the free memory region
                if (start_addr > descending_sorted_mmap_entries[i].addr &&
                    end_addr < descending_sorted_mmap_entries[i].addr +
                                   descending_sorted_mmap_entries[i].length) {
                    // Split the free memory region into two
                    AddFreeMemoryRegion(
                        end_addr, descending_sorted_mmap_entries[i].addr +
                                      descending_sorted_mmap_entries[i].length
                    );
                    descending_sorted_mmap_entries[i].length =
                        start_addr - descending_sorted_mmap_entries[i].addr;
                }
            }
        }
    }
}
void LoaderMemoryManager::DumpMemoryMap()
{
    TRACE_INFO("Memory map dump:");
    for (u32 i = 0; i < num_free_memory_regions_; i++) {
        TRACE_INFO(
            "Region %d: Address: 0x%llX, Length: %llu KB", i,
            descending_sorted_mmap_entries[i].addr, descending_sorted_mmap_entries[i].length << 10
        );
    }
    TRACE_SUCCESS("Memory map dump complete!");
}
void LoaderMemoryManager::DumpPmlTables()
{
    TRACE_INFO("PML tables dump:");
    PML4_t *pml4_table = GetPml4Table();

    u64 *pml4_addr_stack[kNumEntriesPerPml];
    u16 pml4_idx_stack[kNumEntriesPerPml];
    u64 *pml3_addr_stack[kNumEntriesPerPml];
    u16 pml3_idx_stack[kNumEntriesPerPml];
    u64 *pml2_addr_stack[kNumEntriesPerPml];
    u16 pml2_idx_stack[kNumEntriesPerPml];
    u32 pml4_stack_top_idx = 0;
    u32 pml3_stack_top_idx = 0;
    u32 pml2_stack_top_idx = 0;

    auto ReconstructVirtualAddressFromIndices = [](u16 pml4_idx, u16 pml3_idx, u16 pml2_idx,
                                                   u16 pml1_idx) -> u64 {
        return (static_cast<u64>(pml4_idx) << 39) | (static_cast<u64>(pml3_idx) << 30) |
               (static_cast<u64>(pml2_idx) << 21) | (static_cast<u64>(pml1_idx) << 12);
    };

    TRACE_INFO("PML4 table : 0x%X ---------------------------------", pml4_table);
    for (u32 i = 0; i < kNumEntriesPerPml; i++) {
        auto *pml4_entry = reinterpret_cast<PML4Entry *>(&(*pml4_table)[i]);
        if (pml4_entry->present) {
            TRACE_INFO(
                "PML4 entry %u: Present: %llu, Writable: %llu, Frame: 0x%llX", i,
                pml4_entry->present, pml4_entry->writable, pml4_entry->frame << kAddressOffset
            );
            pml4_addr_stack[pml4_stack_top_idx] =
                reinterpret_cast<u64 *>(pml4_entry->frame << kAddressOffset);
            pml4_idx_stack[pml4_stack_top_idx] = i;
            pml4_stack_top_idx++;
        }
    }
    TRACE_INFO("-----------------------------------------------");

    while (pml4_stack_top_idx > 0) {
        pml4_stack_top_idx--;
        auto *pml3_table = reinterpret_cast<PML3Entry *>(pml4_addr_stack[pml4_stack_top_idx]);
        u16 pml4_idx     = pml4_idx_stack[pml4_stack_top_idx];

        TRACE_INFO("PML3 table : 0x%X ---------------------------------", pml3_table);
        for (u32 i = 0; i < kNumEntriesPerPml; i++) {
            if (pml3_table[i].present) {
                if (pml3_table[i].page_size == 1) {
                    TRACE_INFO(
                        "PML3 entry %u: Present: %llu, Writable: %llu, Physical Address (Frame): "
                        "0x%llX, Virtual Address: 0x%llX, Page size: 1GB",
                        i, pml3_table[i].present, pml3_table[i].writable,
                        pml3_table[i].frame << kAddressOffset,
                        ReconstructVirtualAddressFromIndices(pml4_idx, i, 0, 0)
                    );
                } else {
                    TRACE_INFO(
                        "PML3 entry %u: Present: %llu, Writable: %llu, Frame: 0x%llX", i,
                        pml3_table[i].present, pml3_table[i].writable,
                        pml3_table[i].frame << kAddressOffset
                    );
                    pml3_addr_stack[pml3_stack_top_idx] =
                        reinterpret_cast<u64 *>(pml3_table[i].frame << kAddressOffset);
                    pml3_idx_stack[pml3_stack_top_idx] = i;
                    pml3_stack_top_idx++;
                }
            }
        }
        TRACE_INFO("-----------------------------------------------");

        while (pml3_stack_top_idx > 0) {
            pml3_stack_top_idx--;
            auto *pml2_table = reinterpret_cast<PML2Entry *>(pml3_addr_stack[pml3_stack_top_idx]);
            u16 pml3_idx     = pml3_idx_stack[pml3_stack_top_idx];

            TRACE_INFO("PML2 table : 0x%X ---------------------------------", pml2_table);
            for (u32 i = 0; i < kNumEntriesPerPml; i++) {
                if (pml2_table[i].present) {
                    if (pml2_table[i].page_size == 1) {
                        TRACE_INFO(
                            "PML2 entry %u: Present: %llu, Writable: %llu, Physical Address "
                            "(Frame): 0x%llX, Virtual Address: 0x%llX, Page size: 2MB",
                            i, pml2_table[i].present, pml2_table[i].writable,
                            pml2_table[i].frame << kAddressOffset,
                            ReconstructVirtualAddressFromIndices(pml4_idx, pml3_idx, i, 0)
                        );
                    } else {
                        TRACE_INFO(
                            "PML2 entry %u: Present: %llu, Writable: %llu, Frame: 0x%llX", i,
                            pml2_table[i].present, pml2_table[i].writable,
                            pml2_table[i].frame << kAddressOffset
                        );
                        pml2_addr_stack[pml2_stack_top_idx] =
                            reinterpret_cast<u64 *>(pml2_table[i].frame << kAddressOffset);
                        pml2_idx_stack[pml2_stack_top_idx] = i;
                        pml2_stack_top_idx++;
                    }
                }
            }
            TRACE_INFO("-----------------------------------------------");

            while (pml2_stack_top_idx > 0) {
                pml2_stack_top_idx--;
                auto *pml1_table =
                    reinterpret_cast<PML1Entry *>(pml2_addr_stack[pml2_stack_top_idx]);
                u16 pml2_idx = pml2_idx_stack[pml2_stack_top_idx];

                TRACE_INFO("PML1 table : 0x%X ---------------------------------", pml1_table);
                for (u32 i = 0; i < kNumEntriesPerPml; i++) {
                    if (pml1_table[i].present) {
                        TRACE_INFO(
                            "PML1 entry %u: Present: %llu, Writable: %llu, Physical Address "
                            "(Frame): 0x%llX, Virtual Address: 0x%llX, Page size: 4KB",
                            i, pml1_table[i].present, pml1_table[i].writable,
                            pml1_table[i].frame << kAddressOffset,
                            ReconstructVirtualAddressFromIndices(pml4_idx, pml3_idx, pml2_idx, i)
                        );
                    }
                }
                TRACE_INFO("-----------------------------------------------");
            }
        }
    }
}
