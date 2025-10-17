#ifndef ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BITMAP_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BITMAP_HPP_

#include <extensions/bit_array.hpp>
#include <extensions/expected.hpp>

#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

namespace mem
{

template <typename T, typename E>
using Expected = std::expected<T, E>;

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

    BitmapPmm(VirtualPtr<void> mem_bitmap, size_t mem_bitmap_size);
    void Init(BitMapView bmv, size_t last_alloc_idx = 0);

    Expected<PhysicalPtr<Page>, MemError> Alloc(AllocationRequest ar);
    void Free(PhysicalPtr<Page> page, size_t num_pages = 1);

    size_t BitMapSize() const { return bitmap_view_.Size(); }

    private:
    bool IsFree(size_t pfn) const { return bitmap_view_.Get(pfn) == BitMapFree; }
    bool IsAllocated(size_t pfn) const { return bitmap_view_.Get(pfn) == BitMapAllocated; }
    void MarkAllocated(size_t pfn)
    {
        R_ASSERT_TRUE(IsFree(pfn));
        bitmap_view_.Set(pfn, BitMapAllocated);
    }
    void MarkFree(u64 pfn)
    {
        R_ASSERT_TRUE(IsAllocated(pfn));
        bitmap_view_.Set(pfn, BitMapFree);
    }

    BitMapView bitmap_view_{nullptr, 0};
    size_t last_alloc_idx_ = 0;
};

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BITMAP_HPP_
