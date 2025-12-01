#include "mem/phys/mngr/bitmap.hpp"

#include <data_structures/bit_array.hpp>
#include <expected.hpp>
#include "hal/constants.hpp"
#include "mem/page_meta.hpp"
#include "mem/types.hpp"

using namespace Mem;

void BitmapPmm::Init(data_structures::BitMapView bmv)
{
    ASSERT_GT(bmv.Size(), 0UL);
    bitmap_view_    = bmv;
    last_alloc_idx_ = bmv.Size() - 1;
};

Expected<PPtr<Page>, MemError> BitmapPmm::Alloc(AllocationRequest ar)
{
    if (ar.num_pages == 0) {
        return Unexpected(MemError::InvalidArgument);
    }
    const u64 total_pages = bitmap_view_.Size();

    FindBlockResult fbr;
    bool found = false;

    if (last_alloc_idx_ != 0) {
        auto res = FindContiguousBlock(0, last_alloc_idx_, ar.num_pages);
        if (res) {
            fbr   = *res;
            found = true;
        }
    }

    if (!found) {
        auto res = FindContiguousBlock(
            last_alloc_idx_ > ar.num_pages + 1 ? last_alloc_idx_ - ar.num_pages - 1 : 0,
            total_pages, ar.num_pages
        );
        if (res) {
            fbr   = *res;
            found = true;
        }
    }

    if (!found) {
        return Unexpected(MemError::OutOfMemory);
    }

    MarkAllocated(fbr.start_pfn, fbr.start_pfn + ar.num_pages);
    last_alloc_idx_ = fbr.start_pfn;
    return PageFrameAddr(fbr.start_pfn);
}

void BitmapPmm::Free(PPtr<Page> page, size_t num_pages)
{
    size_t pfn = PageFrameNumber(page);

    for (size_t i = 0; i < num_pages; i++) {
        MarkFree(pfn + i);
    }
}

Expected<BitmapPmm::FindBlockResult, MemError> BitmapPmm::FindContiguousBlock(
    size_t range_start_pfn, size_t range_end_pfn, u64 num_pages
)
{
    ASSERT_LT(range_start_pfn, range_end_pfn);
    size_t current_pfn = range_end_pfn - 1;
    u64 count          = 0;

    do {
        if (IsFree(current_pfn)) {
            count++;
        } else {
            count = 0;
        }

        if (count == num_pages) {
            size_t block_start = current_pfn;
            return FindBlockResult{block_start};
        }

    } while (current_pfn-- > range_start_pfn);

    return Unexpected(MemError::OutOfMemory);
}
