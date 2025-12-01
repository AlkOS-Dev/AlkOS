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

FAST_CALL PPtr<Page> PageFrameAddr(size_t pfn)
{
    return UptrToPtr<Page>(static_cast<uptr>(pfn * hal::kPageSizeBytes));
}

/// Given a physical ptr 'a', returns the physical ptr to the start of the page
/// in which 'a' is contained in
template <typename T>
FAST_CALL PPtr<Page> PageFrameAddr(PPtr<T> ptr)
{
    auto *p1 = AlignDown(ptr, hal::kPageSizeBytes);
    auto *p2 = reinterpret_cast<PPtr<Page>>(p1);
    return p2;
}

FAST_CALL size_t PageFrameNumber(PPtr<Page> page)
{
    const uptr addr = PtrToUptr(page);
    ASSERT_TRUE(
        IsAligned(addr, hal::kPageSizeBytes), "Physical page pointer is not aligned to page size"
    );
    return addr / hal::kPageSizeBytes;
}

/// Given a physical ptr 'a', returns the PageFrameNumber of the
/// page in which 'a' is contained in
template <typename T>
FAST_CALL size_t PageFrameNumber(PPtr<T> ptr)
{
    PPtr<Page> p_addr = PageFrameAddr(ptr);
    return PageFrameNumber(p_addr);
}

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PAGE_HPP_
