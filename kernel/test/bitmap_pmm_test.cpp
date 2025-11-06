
#include <test_module/test.hpp>
#include "hal/constants.hpp"
#include "mem/page.hpp"
#include "mem/phys/mngr/bitmap.hpp"

using namespace Mem;
using namespace hal;

class BitmapPmmTest : public TestGroupBase
{
    protected:
    static constexpr size_t kNumPages    = 1024;
    static constexpr size_t kBitmapBytes = kNumPages / 8;

    uint8_t bitmap_buffer_[kBitmapBytes];
    Mem::BitmapPmm pmm_{};

    void Setup_() override
    {
        // Initialize the bitmap to all-free (all zeros) before each test.
        memset(bitmap_buffer_, 0, sizeof(bitmap_buffer_));
        data_structures::BitMapView bmv{bitmap_buffer_, kNumPages};
        pmm_.Init(bmv);
    }
};

// ------------------------------
// Initialization Tests
// ------------------------------

TEST_F(BitmapPmmTest, InitialStateIsCorrect)
{
    // Verifies that after setup, all pages are marked as free.
    EXPECT_EQ(kNumPages, pmm_.BitMapSize());
    // Attempting to allocate all pages should succeed.
    auto page = pmm_.Alloc({.num_pages = kNumPages});
    EXPECT_TRUE(page.has_value());
    EXPECT_EQ(Mem::PageFrameNumber(page.value()), 0);  // Should allocate from the start
}

FAIL_TEST_F(BitmapPmmTest, InitWithZeroPages)
{
    // Re-initialize with an empty bitmap view.
    data_structures::BitMapView bmv{nullptr, 0};
    pmm_.Init(bmv);
}

// ------------------------------
// Single-Page Allocation Tests
// ------------------------------

TEST_F(BitmapPmmTest, AllocReturnsDifferentPages)
{
    auto page1_res = pmm_.Alloc();
    ASSERT_TRUE(page1_res.has_value());

    auto page2_res = pmm_.Alloc();
    ASSERT_TRUE(page2_res.has_value());

    EXPECT_NEQ(page1_res.value(), page2_res.value());
}

TEST_F(BitmapPmmTest, AllocExhaustsMemory)
{
    // Allocate all pages one-by-one.
    for (size_t i = 0; i < kNumPages; ++i) {
        auto page = pmm_.Alloc();
        ASSERT_TRUE(page.has_value(), "Allocation failed unexpectedly at page %zu", i);
    }

    // The next allocation must fail.
    auto final_page = pmm_.Alloc();
    ASSERT_FALSE(final_page.has_value());
    EXPECT_EQ(Mem::MemError::OutOfMemory, final_page.error());
}

// ------------------------------
// Contiguous Block Allocation Tests
// ------------------------------

TEST_F(BitmapPmmTest, AllocContiguousFailsWithFragmentation)
{
    // Allocate all pages to clear state, then free every other page.
    for (size_t i = 0; i < kNumPages; ++i) {
        ASSERT_TRUE(pmm_.Alloc().has_value());
    }
    for (size_t i = 0; i < kNumPages; i += 2) {
        pmm_.Free(Mem::PageFrameAddr(i));
    }

    // Now, enough total pages are free, but no two are contiguous.
    auto contiguous_block = pmm_.Alloc({.num_pages = 2});
    EXPECT_FALSE(contiguous_block.has_value());
    EXPECT_EQ(Mem::MemError::OutOfMemory, contiguous_block.error());

    // But a single page allocation should still succeed.
    auto single_page = pmm_.Alloc();
    EXPECT_TRUE(single_page.has_value());
}

TEST_F(BitmapPmmTest, AllocContiguousFailsWhenRequestingTooManyPages)
{
    auto result = pmm_.Alloc({.num_pages = kNumPages + 1});
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(Mem::MemError::OutOfMemory, result.error());
}

// ------------------------------
// Free Operation Tests
// ------------------------------

FAIL_TEST_F(BitmapPmmTest, DoubleFreeCausesAssert)
{
    auto page_res = pmm_.Alloc();
    ASSERT_TRUE(page_res.has_value());

    pmm_.Free(page_res.value());
    pmm_.Free(page_res.value());  // This should trigger an assertion.
}

FAIL_TEST_F(BitmapPmmTest, FreeUnallocatedPageCausesAssert)
{
    // Attempt to free a page that was never allocated.
    pmm_.Free(Mem::PageFrameAddr(kNumPages / 2));
}

// ------------------------------
// Boundary and Edge Case Tests
// ------------------------------

TEST_F(BitmapPmmTest, AllocZeroPagesFails)
{
    auto result = pmm_.Alloc({.num_pages = 0});
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(Mem::MemError::InvalidArgument, result.error());
}

TEST_F(BitmapPmmTest, AllocAllMemoryAsOneBlock)
{
    auto block = pmm_.Alloc({.num_pages = kNumPages});
    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(Mem::PageFrameNumber(block.value()), 0);

    auto final_page = pmm_.Alloc();
    EXPECT_FALSE(final_page.has_value());
}

// ------------------------------
// Stress and High-Churn Scenarios
// ------------------------------

TEST_F(BitmapPmmTest, HighChurnStressTest)
{
    // A more complex sequence of allocations and deallocations.
    auto p1 = pmm_.Alloc({.num_pages = 10});
    auto p2 = pmm_.Alloc({.num_pages = 5});
    auto p3 = pmm_.Alloc({.num_pages = 20});
    ASSERT_TRUE(p1.has_value() && p2.has_value() && p3.has_value());

    // Free the middle block.
    pmm_.Free(p2.value(), 5);

    // Allocate a smaller block; it should fit in the freed space.
    auto p4 = pmm_.Alloc({.num_pages = 3});
    ASSERT_TRUE(p4.has_value());

    // Allocate another block that also fits.
    auto p5 = pmm_.Alloc({.num_pages = 2});
    ASSERT_TRUE(p5.has_value());

    // Free everything to ensure state is consistent.
    pmm_.Free(p1.value(), 10);
    pmm_.Free(p3.value(), 20);
    pmm_.Free(p4.value(), 3);
    pmm_.Free(p5.value(), 2);

    // After freeing all, a full allocation should be possible again.
    auto final_block = pmm_.Alloc({.num_pages = kNumPages});
    EXPECT_TRUE(final_block.has_value());
}
