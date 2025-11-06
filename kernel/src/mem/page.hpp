#ifndef KERNEL_SRC_MEM_PAGE_HPP_
#define KERNEL_SRC_MEM_PAGE_HPP_

#include <assert.h>
#include <bit.hpp>
#include <types.hpp>

#include "hal/constants.hpp"
#include "mem/types.hpp"

namespace Mem
{

struct Page {
    byte bytes[hal::kPageSizeBytes];
};

FAST_CALL size_t PageFrameNumber(PPtr<Page> page)
{
    const uptr addr = PtrToUptr(page);
    ASSERT_TRUE(
        IsAligned(addr, hal::kPageSizeBytes), "Physical page pointer is not aligned to page size"
    );
    return addr / hal::kPageSizeBytes;
}

FAST_CALL PPtr<Page> PageFrameAddr(size_t pfn)
{
    return UptrToPtr<Page>(static_cast<uptr>(pfn * hal::kPageSizeBytes));
}

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PAGE_HPP_
