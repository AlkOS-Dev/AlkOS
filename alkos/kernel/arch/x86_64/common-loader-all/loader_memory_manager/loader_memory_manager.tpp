#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_TPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_TPP_

#include <assert.h>
#include <string.h>
#include "extensions/debug.hpp"

template <LoaderMemoryManager::PageSize page_size>
void LoaderMemoryManager::MapVirtualMemoryToPhysical(
    u64 virtual_address, u64 physical_address, u64 flags
)
{
    static constexpr u32 kIndexMask = 0x1FF;
    static constexpr i32 kPageShift = (page_size == PageSize::Page4k)   ? 12
                                      : (page_size == PageSize::Page2M) ? 21
                                                                        : 30;

    // Both addresses must be aligned to the page size
    R_ASSERT(IsAligned(physical_address, 1 << kPageShift));
    R_ASSERT(IsAligned(virtual_address, 1 << kPageShift));

    // Calculate the indexes for each level of the page table
    const u32 pml4_index = (virtual_address >> 39) & kIndexMask;
    const u32 pml3_index = (virtual_address >> 30) & kIndexMask;
    const u32 pml2_index = (virtual_address >> 21) & kIndexMask;
    const u32 pml1_index = (virtual_address >> 12) & kIndexMask;

    // Ensure PML4 entry points to the correct PDPT
    PML4_t *pml4_table = GetPml4Table();
    ASSERT_NOT_NULL(pml4_table);

    auto *pml4_entry = reinterpret_cast<PML4Entry *>(&(*pml4_table)[pml4_index]);
    if (!pml4_entry->present) {
        // Allocate a new PML3 table
        ASSERT(num_pml_tables_stored_ < kMaxPmlTablesToStore);
        auto *pml3_table = reinterpret_cast<PML3_t *>(&buffer_[num_pml_tables_stored_++]);
        memset(pml3_table, 0, sizeof(PML3_t));
        pml4_entry->frame    = reinterpret_cast<u64>(pml3_table) >> kAddressOffset;
        pml4_entry->present  = 1;
        pml4_entry->writable = 1;
    }

    auto pml3_table = reinterpret_cast<PML3Entry *>(pml4_entry->frame << kAddressOffset);

    if constexpr (page_size == PageSize::Page1G) {
        // PML3 Level
        auto casted_pml3_table                  = reinterpret_cast<PML3Entry1GB *>(pml3_table);
        casted_pml3_table[pml3_index].present   = 1;
        casted_pml3_table[pml3_index].writable  = 1;
        casted_pml3_table[pml3_index].page_size = 1;
        casted_pml3_table[pml3_index].frame     = physical_address >> kPageShift;
        u64 *entry = reinterpret_cast<u64 *>(&casted_pml3_table[pml3_index]);
        *entry |= flags;
        return;
    }

    if (!pml3_table[pml3_index].present) {
        // Allocate a new PML2 table
        ASSERT(num_pml_tables_stored_ < kMaxPmlTablesToStore);
        auto *pml2_table = reinterpret_cast<PML2_t *>(&buffer_[num_pml_tables_stored_++]);
        memset(pml2_table, 0, sizeof(PML2_t));
        pml3_table[pml3_index].frame    = reinterpret_cast<u64>(pml2_table) >> kAddressOffset;
        pml3_table[pml3_index].present  = 1;
        pml3_table[pml3_index].writable = 1;
    }

    auto pml2_table = reinterpret_cast<PML2Entry *>(pml3_table[pml3_index].frame << kAddressOffset);

    if constexpr (page_size == PageSize::Page2M) {
        // PML2 Level
        auto casted_pml2_table                  = reinterpret_cast<PML2Entry2MB *>(pml2_table);
        casted_pml2_table[pml2_index].present   = 1;
        casted_pml2_table[pml2_index].writable  = 1;
        casted_pml2_table[pml2_index].page_size = 1;
        casted_pml2_table[pml2_index].frame     = physical_address >> kPageShift;
        u64 *entry = reinterpret_cast<u64 *>(&casted_pml2_table[pml2_index]);
        *entry |= flags;
        return;
    }

    if (!pml2_table[pml2_index].present) {
        // Allocate a new PML1 table
        ASSERT(num_pml_tables_stored_ < kMaxPmlTablesToStore);
        auto *pml1_table = reinterpret_cast<PML1_t *>(&buffer_[num_pml_tables_stored_++]);
        memset(pml1_table, 0, sizeof(PML1_t));
        pml2_table[pml2_index].frame    = reinterpret_cast<u64>(pml1_table) >> kAddressOffset;
        pml2_table[pml2_index].present  = 1;
        pml2_table[pml2_index].writable = 1;
    }

    auto p1_entry = reinterpret_cast<PML1Entry *>(pml2_table[pml2_index].frame << kAddressOffset);
    p1_entry[pml1_index].present  = 1;
    p1_entry[pml1_index].writable = 1;
    p1_entry[pml1_index].frame    = physical_address >> kPageShift;
    u64 *entry                    = reinterpret_cast<u64 *>(&p1_entry[pml1_index]);
    *entry |= flags;
}

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_TPP_
