#include "mem/phys/mngr/bitmap.hpp"

#include <extensions/bit_array.hpp>
#include <extensions/expected.hpp>

#include "hal/constants.hpp"

using namespace mem;

std::expected<PhysicalPtr<Page>, MemError> BitmapPmm::Alloc(AllocationRequest ar)
{
    const u64 total_pages = bitmap_view_.Size();

    for (u64 i = 0; i < total_pages;) {
        const u64 start_idx = (last_alloc_idx_ + i) % total_pages;

        if (!IsFree(start_idx)) {
            i++;
            continue;
        }

        // Try to find a contiguous block of the required size
        u64 block_end_idx = start_idx + 1;
        while ((block_end_idx < total_pages) && (block_end_idx - start_idx < ar.num_pages) &&
               IsFree(block_end_idx)) {
            block_end_idx++;
        }

        const u64 found_pages = block_end_idx - start_idx;

        // Not found
        if (found_pages != ar.num_pages) {
            i += found_pages + 1;
            continue;
        }

        // Found
        for (u64 pfn = start_idx; pfn < block_end_idx; ++pfn) {
            MarkAllocated(pfn);
        }

        last_alloc_idx_ = block_end_idx % total_pages;
        return PageFrameAddr(start_idx);  // Return the START of the block
    }

    return std::unexpected(MemError::OutOfMemory);
}

void BitmapPmm::Free(PhysicalPtr<Page> page, size_t num_pages)
{
    size_t pfn = PageFrameNumber(page);

    for (size_t i = 0; i < num_pages; i++) {
        // Use the MarkFree helper to leverage its internal assertion
        MarkFree(pfn + i);
    }
}
