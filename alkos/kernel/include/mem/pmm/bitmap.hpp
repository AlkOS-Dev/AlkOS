#ifndef ALKOS_KERNEL_INCLUDE_MEM_PMM_BITMAP_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PMM_BITMAP_HPP_

#include <extensions/bit_array.hpp>
#include <extensions/expected.hpp>

#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/phys_ptr.hpp"

namespace mem
{

class BitmapPmm
{
    public:
    enum {
        BitMapFree      = 0,
        BitMapAllocated = 1,
    };

    enum class Zone { B64, B32, B16 };

    std::expected<PhysicalPtr<Page>, MemError> Alloc();
    void Free(PhysicalPtr<Page> page);

    BitMapView bitmap_view_{nullptr, 0};
    u64 last_alloc_idx_ = 0;
};

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PMM_BITMAP_HPP_
