#ifndef ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BITMAP_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BITMAP_HPP_

#include <extensions/data_structures/bit_array.hpp>
#include <extensions/expected.hpp>

#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/types.hpp"

namespace Mem
{

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

    BitmapPmm(VPtr<void> mem_bitmap, size_t mem_bitmap_size);
    void Init(data_structures::BitMapView bmv, size_t last_alloc_idx = 0);

    Expected<PPtr<Page>, MemError> Alloc(AllocationRequest ar);
    void Free(PPtr<Page> page, size_t num_pages = 1);

    size_t BitMapSize() const { return bitmap_view_.Size(); }

    private:
    bool IsFree(size_t pfn) const { return bitmap_view_.Get(pfn) == BitMapFree; }
    bool IsAllocated(size_t pfn) const { return bitmap_view_.Get(pfn) == BitMapAllocated; }
    void MarkAllocated(size_t pfn)
    {
        ASSERT_TRUE(IsFree(pfn), "Page frame is already allocated");
        bitmap_view_.Set(pfn, BitMapAllocated);
    }
    void MarkFree(u64 pfn)
    {
        ASSERT_TRUE(IsAllocated(pfn), "Page frame is already free");
        bitmap_view_.Set(pfn, BitMapFree);
    }

    data_structures::BitMapView bitmap_view_{nullptr, 0};
    size_t last_alloc_idx_ = 0;
};

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BITMAP_HPP_
