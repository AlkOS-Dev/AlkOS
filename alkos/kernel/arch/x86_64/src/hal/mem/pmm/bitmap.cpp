#include <assert.h>
#include <extensions/bit_array.hpp>

#include "hal/mem/pmm/bitmap.hpp"

using namespace arch;

std::expected<PhysicalPtr<Page>, MemError> BitmapPmm::Alloc(AllocationRequest req)
{
    ASSERT_LE(req.num_pages, 1u, "Bootstrap BitmapPmm does not support contiguous allocation");

    const u64 total_pages = bitmap_view_.Size();
    for (u64 i = 0; i < total_pages; ++i) {
        u64 current_idx = (last_alloc_idx_ + i) % total_pages;

        if (bitmap_view_.Get(current_idx) == BitMapFree) {
            bitmap_view_.Set(current_idx, BitMapAllocated);
            last_alloc_idx_ = current_idx + 1;
            auto* page_addr = reinterpret_cast<Page*>(current_idx * kPageSizeBytes);

            if (req.flags == AllocFlags::kZeroed) {
                memset(PhysicalPtr<Page>(page_addr).ToVirt(), 0, kPageSizeBytes);
            }
            return PhysicalPtr<Page>(page_addr);
        }
    }
    return std::unexpected(MemError::OutOfMemory);
}

void BitmapPmm::Free(PhysicalPtr<Page> /*page*/, u64 /*num_pages*/)
{
    FAIL_ALWAYS("Free() called on bootstrap PMM. This is not supported.");
}

void BitmapPmm::InitImpl()
{
    const BitmapPmmConfig& c = GetConfig();
    bitmap_view_             = BitMapView(c.pmm_bitmap_addr.ToVirt(), c.pmm_total_pages);
}
