#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_TPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_TPP_

#include <assert.h>
#include <memory.h>
#include "extensions/debug.hpp"

namespace memory
{

template <FreeRegionProvider Provider, LoaderMemoryManager::WalkDirection direction>
void LoaderMemoryManager::MapVirtualRangeUsingFreeRegionProvider(
    Provider provider, u64 virtual_address, u64 size_bytes, u64 flags
)
{
    constexpr bool is_descending          = direction == WalkDirection::Descending;
    static constexpr u32 k4kPageSizeBytes = 1 << 12;

    ASSERT(IsAligned(virtual_address, k4kPageSizeBytes));
    ASSERT_GE(available_memory_bytes_, size_bytes);

    u64 mapped_bytes = 0;

    provider([&](FreeMemoryRegion_t &region) {
        if (mapped_bytes >= size_bytes) {
            return;
        }

        // Skip exhausted memory regions
        if (region.length < k4kPageSizeBytes) {
            return;
        }

        const u64 current_physical_address =
            is_descending ? region.addr + region.length - k4kPageSizeBytes : region.addr;
        const u64 offset_from_alignment =
            is_descending
                ? current_physical_address - AlignDown(current_physical_address, k4kPageSizeBytes)
                : AlignUp(current_physical_address, k4kPageSizeBytes) - current_physical_address;

        if (offset_from_alignment > 0) {
            if (region.length < offset_from_alignment + k4kPageSizeBytes) {
                return;
            }
            UsePartOfFreeMemoryRegion<direction>(region, offset_from_alignment);
        }

        // Try to map as many pages as possible from the current memory region
        while (region.length >= k4kPageSizeBytes && mapped_bytes < size_bytes) {
            const u64 current_virtual_address = virtual_address + mapped_bytes;
            const u64 physical_address_to_map =
                is_descending ? (region.addr + region.length - k4kPageSizeBytes) : region.addr;
            MapVirtualMemoryToPhysical<PageSize::Page4k>(
                current_virtual_address, physical_address_to_map, flags
            );
            UsePartOfFreeMemoryRegion<direction>(region, k4kPageSizeBytes);
            mapped_bytes += k4kPageSizeBytes;
        }
    });

    if (mapped_bytes < size_bytes) {
        KernelPanic("Failed to map virtual memory range using internal memory map - out of memory!"
        );
    }
}

template <LoaderMemoryManager::WalkDirection direction>
void LoaderMemoryManager::MapVirtualRangeUsingInternalMemoryMap(
    u64 virtual_address, u64 size_bytes, u64 flags
)
{
    const auto internal_provider = [this](auto callback) {
        WalkFreeMemoryRegions<direction>(callback);
    };
    MapVirtualRangeUsingFreeRegionProvider<decltype(internal_provider), direction>(
        internal_provider, virtual_address, size_bytes, flags
    );
}

template <LoaderMemoryManager::WalkDirection direction>
void LoaderMemoryManager::MapVirtualRangeUsingExternalMemoryMap(
    multiboot::tag_mmap_t *mmap_tag, u64 virtual_address, u64 size_bytes, u64 flags
)
{
    using namespace multiboot;
    R_ASSERT_NOT_NULL(mmap_tag);

    uintptr_t descending_sorted_address_buffer[kMaxMemoryMapEntries];
    u64 address_count = 0;
    WalkMemoryMap(mmap_tag, [&](multiboot::memory_map_t *entry) {
        if (entry->type != multiboot::mmap_entry_t::kMemoryAvailable) {
            return;
        }
        descending_sorted_address_buffer[address_count++] = reinterpret_cast<uintptr_t>(entry);
        u64 i                                             = address_count - 1;
        while (i > 0 &&
               descending_sorted_address_buffer[i] > descending_sorted_address_buffer[i - 1]) {
            uintptr_t tmp                           = descending_sorted_address_buffer[i - 1];
            descending_sorted_address_buffer[i - 1] = descending_sorted_address_buffer[i];
            descending_sorted_address_buffer[i]     = tmp;
            i--;
        }
    });

    const auto external_provider = [&](auto callback) {
        if constexpr (direction == WalkDirection::Descending) {
            for (u64 i = 0; i < address_count; i++) {
                auto *entry = reinterpret_cast<mmap_entry_t *>(descending_sorted_address_buffer[i]);
                FreeMemoryRegion_t region{entry->addr, entry->len};
                callback(region);
                entry->addr = region.addr;
                entry->addr = region.length;
            }
        } else {
            for (u64 i = address_count - 1; i > 0; i--) {
                auto *entry = reinterpret_cast<mmap_entry_t *>(descending_sorted_address_buffer[i]);
                FreeMemoryRegion_t region{entry->addr, entry->len};
                callback(region);
                entry->addr = region.addr;
                entry->addr = region.length;
            }
        }
    };
    MapVirtualRangeUsingFreeRegionProvider<decltype(external_provider), direction>(
        external_provider, virtual_address, size_bytes, flags
    );
}

template <PageSize page_size>
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

    auto *pml4_entry = reinterpret_cast<PML4Entry_t *>(&(*pml4_table)[pml4_index]);
    if (!pml4_entry->present) {
        // Allocate a new PML3 table
        ASSERT(num_pml_tables_stored_ < kMaxPmlTablesToStore);
        auto *pml3_table = reinterpret_cast<PML3_t *>(&buffer_[num_pml_tables_stored_++]);
        memset(pml3_table, 0, sizeof(PML3_t));
        pml4_entry->frame    = reinterpret_cast<u64>(pml3_table) >> kAddressOffset;
        pml4_entry->present  = 1;
        pml4_entry->writable = 1;
    }

    auto pml3_table = reinterpret_cast<PML3Entry_t *>(pml4_entry->frame << kAddressOffset);

    if constexpr (page_size == PageSize::Page1G) {
        // PML3 Level
        auto casted_pml3_table                  = reinterpret_cast<PML3Entry1GB_t *>(pml3_table);
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

    auto pml2_table =
        reinterpret_cast<PML2Entry_t *>(pml3_table[pml3_index].frame << kAddressOffset);

    if constexpr (page_size == PageSize::Page2M) {
        // PML2 Level
        auto casted_pml2_table                  = reinterpret_cast<PML2Entry2MB_t *>(pml2_table);
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

    auto p1_entry = reinterpret_cast<PML1Entry_t *>(pml2_table[pml2_index].frame << kAddressOffset);
    p1_entry[pml1_index].present  = 1;
    p1_entry[pml1_index].writable = 1;
    p1_entry[pml1_index].frame    = physical_address >> kPageShift;
    u64 *entry                    = reinterpret_cast<u64 *>(&p1_entry[pml1_index]);
    *entry |= flags;
}
template <LoaderMemoryManager::WalkDirection direction, FreeMemoryRegionCallback Callback>
void LoaderMemoryManager::WalkFreeMemoryRegions(Callback callback)
{
    TRACE_INFO("Walking free memory regions...");
    switch (direction) {
        case WalkDirection::Ascending: {
            for (u32 i = num_free_memory_regions_; i > 0; i--) {
                callback(descending_sorted_mmap_entries[i - 1]);
            }
            break;
        }
        case WalkDirection::Descending: {
            for (u32 i = 0; i < num_free_memory_regions_; i++) {
                callback(descending_sorted_mmap_entries[i]);
            }
            break;
        }
    }
    TRACE_INFO("Free memory regions walk complete!");
}
template <LoaderMemoryManager::WalkDirection direction>
void LoaderMemoryManager::UsePartOfFreeMemoryRegion(FreeMemoryRegion_t &region, u64 size_bytes)
{
    ASSERT_GE(region.length, size_bytes);

    switch (direction) {
        case WalkDirection::Descending: {
            region.length -= size_bytes;
            available_memory_bytes_ -= size_bytes;
            break;
        }
        case WalkDirection::Ascending: {
            region.addr += size_bytes;
            region.length -= size_bytes;
            available_memory_bytes_ -= size_bytes;
            break;
        }
    }
}

}  // namespace memory
#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_TPP_
