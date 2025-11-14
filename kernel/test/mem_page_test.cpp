#include <test_module/test.hpp>

#include <hal/constants.hpp>
#include <limits.hpp>
#include "mem/page.hpp"
#include "mem/types.hpp"

namespace
{
using namespace Mem;
using namespace hal;

TEST(PageFrameAddr_GivenZeroPfn_ReturnsPointerToStartOfPhysicalMemory)
{
    const auto ptr = PageFrameAddr(0);
    R_ASSERT_EQ(PtrToUptr(ptr), 0);
}

TEST(PageFrameAddr_GivenValidPfn_ReturnsCorrectlyCalculatedAddress)
{
    constexpr size_t pfn = 10;
    const auto ptr       = PageFrameAddr(pfn);
    R_ASSERT_NOT_NULL(ptr);
    R_ASSERT_EQ(PtrToUptr(ptr), pfn * kPageSizeBytes);
}

TEST(PageFrameAddr_GivenMaximumValidPfn_ReturnsCorrectAddressWithoutOverflow)
{
    constexpr size_t pfn = std::numeric_limits<size_t>::max() / kPageSizeBytes;
    const auto ptr       = PageFrameAddr(pfn);
    R_ASSERT_NOT_NULL(ptr);
    R_ASSERT_EQ(PtrToUptr(ptr), pfn * kPageSizeBytes);
}

TEST(PageFrameAddr_FromPointerAtPageStart_ReturnsSameAddress)
{
    const auto ptr      = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * 5));
    const auto page_ptr = PageFrameAddr(ptr);
    R_ASSERT_EQ(PtrToUptr(page_ptr), PtrToUptr(ptr));
}

TEST(PageFrameAddr_FromPointerWithinPage_ReturnsStartOfContainingPage)
{
    const auto ptr      = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * 5 + 100));
    const auto page_ptr = PageFrameAddr(ptr);
    R_ASSERT_EQ(PtrToUptr(page_ptr), kPageSizeBytes * 5);
}

TEST(PageFrameAddr_FromPointerAtPageEnd_ReturnsStartOfContainingPage)
{
    const auto ptr      = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * 3 - 1));
    const auto page_ptr = PageFrameAddr(ptr);
    R_ASSERT_EQ(PtrToUptr(page_ptr), kPageSizeBytes * 2);
}

TEST(PageFrameAddr_FromNullPointer_ReturnsNullPointer)
{
    const auto ptr      = PPtr<int>(nullptr);
    const auto page_ptr = PageFrameAddr(ptr);
    R_ASSERT_NULL(page_ptr);
}

TEST(PageFrameNumber_FromPointer_ReturnsCorrectPfn)
{
    constexpr size_t pfn  = 12;
    const auto ptr        = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * pfn + 100));
    const auto result_pfn = PageFrameNumber(ptr);
    R_ASSERT_EQ(result_pfn, pfn);
}

TEST(PageFrameNumber_FromPointerToPageBoundary_ReturnsCorrectPfn)
{
    constexpr size_t pfn  = 42;
    const auto ptr        = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * pfn));
    const auto result_pfn = PageFrameNumber(ptr);
    R_ASSERT_EQ(result_pfn, pfn);
}

TEST(PageFrameNumber_FromNullPointer_ReturnsPfnZero)
{
    const auto ptr        = PPtr<int>(nullptr);
    const auto result_pfn = PageFrameNumber(ptr);
    R_ASSERT_ZERO(result_pfn);
}

}  // namespace
