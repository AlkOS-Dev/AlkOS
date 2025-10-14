#include "mem/pmm/bitmap.hpp"

#include <extensions/bit_array.hpp>
#include <extensions/expected.hpp>

#include "hal/constants.hpp"

using namespace mem;

std::expected<PhysicalPtr<Page>, MemError> BitmapPmm::Alloc()
{
    const u64 total_pages = bitmap_view_.Size();
    for (u64 i = 0; i < total_pages; ++i) {
        u64 current_idx = (last_alloc_idx_ + i) % total_pages;

        if (bitmap_view_.Get(current_idx) == BitMapFree) {
            bitmap_view_.Set(current_idx, BitMapAllocated);
            last_alloc_idx_ = current_idx + 1;
            auto* page_addr = reinterpret_cast<Page*>(current_idx * hal::kPageSizeBytes);

            return PhysicalPtr<Page>(page_addr);
        }
    }
    return std::unexpected(MemError::OutOfMemory);
}

void BitmapPmm::Free(PhysicalPtr<Page> page)
{
    size_t pn = GetPageNumber(page);

    R_ASSERT_EQ(bitmap_view_.Get(pn), BitMapAllocated);
    bitmap_view_.Set(pn, BitMapFree);
};
