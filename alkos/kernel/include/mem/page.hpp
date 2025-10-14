#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_

#include <extensions/bit.hpp>

#include "hal/constants.hpp"
#include "mem/phys_ptr.hpp"

namespace mem
{

struct Page {
    byte bytes[hal::kPageSizeBytes];
};

FAST_CALL size_t GetPageNumber(PhysicalPtr<Page> page)
{
    const u64 page_num =
        AlignDown(static_cast<u64>(page.AsUIntPtr()), hal::kPageSizeBytes) / hal::kPageSizeBytes;
    return page_num;
}

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_HPP_
