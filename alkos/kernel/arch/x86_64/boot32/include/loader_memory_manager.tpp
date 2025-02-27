#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_LOADER_MEMORY_MANAGER_TPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_LOADER_MEMORY_MANAGER_TPP_

#include <assert.h>
#include <memory.h>
#include "debug.hpp"

template <LoaderMemoryManager::PageSize page_size>
void LoaderMemoryManager::MapVirtualMemoryToPhysical(
    u32 virtual_address_lower, u32 virtual_address_upper, u32 physical_address_lower,
    u32 physical_address_upper, u64 flags
)
{
    static constexpr u32 kIndexMask     = 0x1FF;
    static constexpr i32 kPageShift     = (page_size == PageSize::Page4k)   ? 12
                                          : (page_size == PageSize::Page2M) ? 21
                                                                            : 30;
    static constexpr u64 kAlignmentMask = (1ULL << kPageShift) - 1;

    R_ASSERT_ZERO(virtual_address_lower & kAlignmentMask);  // Virtual address must be page aligned
    R_ASSERT_ZERO(
        physical_address_lower & kAlignmentMask
    );  // Physical address must be page aligned

    u64 phys_addr = (static_cast<u64>(physical_address_upper) << 32) | physical_address_lower;

    // Calculate the indexes for each level of the page table
    u32 pml4_index = (virtual_address_upper >> (39 - 32)) & kIndexMask;
    u32 pml3_index = (virtual_address_lower >> 30) & kIndexMask;
    u32 pml2_index = (virtual_address_lower >> 21) & kIndexMask;
    u32 pml1_index = (virtual_address_lower >> 12) & kIndexMask;

    TRACE_INFO(
        "Mapping virtual address: 0x%08x%08x to physical address: 0x%08x%08x with flags: 0x%08x",
        virtual_address_upper, virtual_address_lower, physical_address_upper,
        physical_address_lower, flags
    );

    TRACE_INFO(
        "PML4 Index: %d, PML3 Index: %d, PML2 Index: %d, PML1 Index: %d", pml4_index, pml3_index,
        pml2_index, pml1_index
    );

    // Ensure PML4 entry points to the correct PDPT
    PML4_t *pml4_table = GetPml4Table();
    ASSERT_NOT_NULL(pml4_table);

    auto *pml4_entry = reinterpret_cast<PML4Entry *>(&pml4_table[pml4_index]);
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
        casted_pml3_table[pml3_index].frame     = phys_addr >> kPageShift;
        u64 *entry = reinterpret_cast<u64 *>(&casted_pml3_table[pml3_index]);
        *entry |= flags;
        TRACE_INFO("PML3 Entry: 0x%016x", *entry);
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
        casted_pml2_table[pml2_index].frame     = phys_addr >> kPageShift;
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
    p1_entry[pml1_index].frame    = phys_addr >> kPageShift;
    u64 *entry                    = reinterpret_cast<u64 *>(&p1_entry[pml1_index]);
    *entry |= flags;
}

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_LOADER_MEMORY_MANAGER_TPP_
