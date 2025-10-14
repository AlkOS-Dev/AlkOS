#ifndef ALKOS_KERNEL_INCLUDE_MEM_PMM_BITMAP_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PMM_BITMAP_HPP_

#ifndef aj23rhj2hrj32
#define aj23rhj2hrj32

#include <extensions/expected.hpp>

#include "mem/error.hpp"

class BitmapPmm
{
    public:
    enum {
        BitMapFree      = 0,
        BitMapAllocated = 1,
    };

    enum class Zone { B64, B32, B16 };

    struct AllocationRequest {
        u64 count;
        Zone z;
    };

    std::expected<PhysicalPtr<Page>, MemError> Alloc(const AllocationRequest& req = {});
    void Free(PhysicalPtr<Page> page, u64 num_pages);

    BitMapView bitmap_view_{nullptr, 0};
    u64 last_alloc_idx_ = 0;
};

#endif  // aj23rhj2hrj32

#endif /* ALKOS_KERNEL_INCLUDE_MEM_PMM_BITMAP_HPP_ */
