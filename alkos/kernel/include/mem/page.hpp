#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_

#include <assert.h>
#include <extensions/bit.hpp>
#include <extensions/types.hpp>

#include "hal/constants.hpp"
#include "mem/phys/ptr.hpp"

namespace mem
{

struct Page {
    byte bytes[hal::kPageSizeBytes];
};

using pfn_t = size_t;

FAST_CALL pfn_t PageFrameNumber(PhysicalPtr<Page> page)
{
    const uptr addr = page.AsUIntPtr();
    R_ASSERT_TRUE(IsAligned(addr, hal::kPageSizeBytes));
    return addr / hal::kPageSizeBytes;
}

FAST_CALL PhysicalPtr<Page> PageFrameAddr(pfn_t pfn)
{
    return PhysicalPtr<Page>(static_cast<uptr>(pfn * hal::kPageSizeBytes));
}

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_
