#ifndef KERNEL_SRC_MEM_PHYS_MNGR_BITMAP_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_BITMAP_HPP_

#include <data_structures/bit_array.hpp>
#include <expected.hpp>

#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/types.hpp"

namespace Mem
{

using std::expected;
using std::unexpected;

class BuddyPmm;

class BitmapPmm
{
    public:
    enum {
        BitMapFree      = 0,
        BitMapAllocated = 1,
    };

    struct AllocationRequest {
        size_t num_pages = 1;
    };

    BitmapPmm() = default;
    void Init(data_structures::BitMapView bmv);

    expected<PPtr<Page>, MemError> Alloc(AllocationRequest ar = {.num_pages = 1});
    void Free(PPtr<Page> page, size_t num_pages = 1);

    size_t BitMapSize() const { return bitmap_view_.Size(); }

    private:
    struct FindBlockResult {
        size_t start_pfn;
    };

    expected<FindBlockResult, MemError> FindContiguousBlock(
        size_t start_range_pfn, size_t end_range_pfn, u64 num_pages
    );
    bool IsFree(size_t pfn) const { return bitmap_view_.Get(pfn) == BitMapFree; }
    bool IsAllocated(size_t pfn) const { return bitmap_view_.Get(pfn) == BitMapAllocated; }
    void MarkAllocated(size_t pfn)
    {
        ASSERT_TRUE(IsFree(pfn), "Page frame is already allocated");
        bitmap_view_.Set(pfn, BitMapAllocated);
    }
    void MarkAllocated(size_t start_range_pfn, size_t end_range_pfn)
    {
        ASSERT_LT(start_range_pfn, end_range_pfn);
        for (size_t i = start_range_pfn; i < end_range_pfn; i++) {
            MarkAllocated(i);
        }
    }
    void MarkFree(u64 pfn)
    {
        ASSERT_TRUE(IsAllocated(pfn), "Page frame is already free");
        bitmap_view_.Set(pfn, BitMapFree);
    }
    /// Private accessor for friend (buddy pmm)
    data_structures::BitMapView &GetBitmapView() { return bitmap_view_; }

    data_structures::BitMapView bitmap_view_{nullptr, 0};
    size_t last_alloc_idx_ = 0;

    /// Friends
    friend BuddyPmm;
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PHYS_MNGR_BITMAP_HPP_
