#include "mem/phys/mngr/bitmap.hpp"

#include <extensions/data_structures/bit_array.hpp>
#include <extensions/expected.hpp>
#include "hal/constants.hpp"
#include "mem/page_meta.hpp"
#include "mem/types.hpp"

using namespace Mem;

BitmapPmm::BitmapPmm(VPtr<void> mem_bitmap, size_t mem_bitmap_size)
{
    ASSERT_NOT_NULL(mem_bitmap, "Memory bitmap pointer is null");
    data_structures::BitMapView bmv{mem_bitmap, static_cast<size_t>(mem_bitmap_size)};
    Init(bmv);
};

void BitmapPmm::Init(data_structures::BitMapView bmv, size_t last_alloc_idx)
{
    bitmap_view_    = bmv;
    last_alloc_idx_ = last_alloc_idx;
};

Expected<PPtr<Page>, MemError> BitmapPmm::Alloc(AllocationRequest ar)
{
    const u64 total_pages = bitmap_view_.Size();

    // Iterate from the end of the bitmap to the beginning.
    for (i64 i = total_pages - ar.num_pages; i >= 0;) {
        const u64 end_idx = i + ar.num_pages - 1;

        // Check if the potential end of the block is free.
        if (!IsFree(end_idx)) {
            i--;
            continue;
        }

        // Try to find a contiguous block of the required size, searching backwards.
        u64 block_start_idx = end_idx;
        while ((block_start_idx > 0) && (end_idx + 1 - block_start_idx < ar.num_pages) &&
               IsFree(block_start_idx - 1)) {
            block_start_idx--;
        }

        const u64 found_pages = end_idx - block_start_idx + 1;

        // Not found
        if (found_pages != ar.num_pages) {
            // Jump the index past the checked non-contiguous block.
            i = block_start_idx - 1;
            continue;
        }

        // Found
        for (u64 pfn = block_start_idx; pfn <= end_idx; ++pfn) {
            MarkAllocated(pfn);
        }

        // Update last_alloc_idx_ to the start of the allocated block for potential future
        // optimizations.
        last_alloc_idx_ = block_start_idx;
        return PageFrameAddr(block_start_idx);  // Return the START of the block
    }

    return Unexpected(MemError::OutOfMemory);
}

void BitmapPmm::Free(PPtr<Page> page, size_t num_pages)
{
    size_t pfn = PageFrameNumber(page);

    for (size_t i = 0; i < num_pages; i++) {
        MarkFree(pfn + i);
    }
}
