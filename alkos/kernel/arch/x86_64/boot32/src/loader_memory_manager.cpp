#include "loader_memory_manager.hpp"
#include "assert.h"
#include "debug.hpp"
#include "extensions/new.hpp"
#include "memory.h"

// Note: The alignment here is a strict requirement for the PML tables and if the
// initial object is not aligned, the PML tables will not be aligned either.
byte kLoaderPreAllocatedMemory[sizeof(LoaderMemoryManager)] __attribute__((aligned(4096)));

LoaderMemoryManager::LoaderMemoryManager()
{
    num_pml_tables_stored_ = 1;  ///< The first PML table is the PML4 table
    memset(buffer_, 0, sizeof(buffer_));
    memset(descending_sorted_mmap_entries, 0, sizeof(descending_sorted_mmap_entries));
}
LoaderMemoryManager::PML4_t *LoaderMemoryManager::GetPml4Table() { return &buffer_[kPml4Index]; }
void LoaderMemoryManager::MapVirtualRangeUsingInternalMemoryMap(
    u64 virtual_address, u64 size_bytes, u64 flags
)
{
    static constexpr u32 k4kPageSizeBytes     = 1 << 12;
    static constexpr u32 k4kPageAlignmentMask = k4kPageSizeBytes - 1;

    ASSERT_ZERO(virtual_address & k4kPageAlignmentMask);  // Virtual address must be page aligned
    ASSERT_GE(available_memory_bytes_, size_bytes);

    u64 mapped_bytes = 0;
    while (mapped_bytes < size_bytes) {
        bool mapped_this_iteration = false;
        for (u32 i = 0; i < num_free_memory_regions_; i++) {
            // Skip exhausted memory regions
            if (descending_sorted_mmap_entries[i].length < k4kPageSizeBytes) {
                continue;
            }

            u64 current_physical_address = descending_sorted_mmap_entries[i].addr;
            u64 aligned_physical_address =
                (current_physical_address + k4kPageAlignmentMask) & ~k4kPageAlignmentMask;
            u32 offset_from_alignment = aligned_physical_address - current_physical_address;

            if (offset_from_alignment > 0) {
                if (descending_sorted_mmap_entries[i].length < offset_from_alignment) {
                    continue;
                }
                descending_sorted_mmap_entries[i].addr += offset_from_alignment;
                descending_sorted_mmap_entries[i].length -= offset_from_alignment;
                available_memory_bytes_ -= offset_from_alignment;
            }

            // Try to map as many pages as possible from the current memory region
            while (descending_sorted_mmap_entries[i].length >= k4kPageSizeBytes &&
                   mapped_bytes < size_bytes) {
                u64 current_virtual_address = virtual_address + mapped_bytes;
                MapVirtualMemoryToPhysical<PageSize::Page4k>(
                    current_virtual_address, descending_sorted_mmap_entries[i].addr, flags
                );
                mapped_this_iteration = true;

                // Update the region: move the start address forward by one page and reduce its
                // length.
                descending_sorted_mmap_entries[i].addr += k4kPageSizeBytes;
                descending_sorted_mmap_entries[i].length -= k4kPageSizeBytes;
                mapped_bytes += k4kPageSizeBytes;
                available_memory_bytes_ -= k4kPageSizeBytes;
            }
        }
        if (mapped_bytes >= size_bytes) {
            break;
        }
        // If no memory region was mapped in this iteration, we are out of memory
        if (!mapped_this_iteration) {
            break;
        }
    }
    if (mapped_bytes < size_bytes) {
        KernelPanic("Failed to map virtual memory range using internal memory map - out of memory!"
        );
    }
}
void LoaderMemoryManager::AddMemoryMapEntry(multiboot::memory_map_t *mmap_entry)
{
    R_ASSERT_LT(num_free_memory_regions_, kMaxMemoryMapEntries);
    FreeMemoryRegion_t &mmap_entry_ref = descending_sorted_mmap_entries[num_free_memory_regions_++];
    mmap_entry_ref.addr                = mmap_entry->addr;
    mmap_entry_ref.length              = mmap_entry->len;
    available_memory_bytes_ += mmap_entry->len;

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
    }
}
