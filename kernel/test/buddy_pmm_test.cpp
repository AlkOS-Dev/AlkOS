#include <test_module/test.hpp>

#include <array.hpp>
#include <data_structures/data_structures.hpp>
#include <span.hpp>
#include <tuple.hpp>

#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/phys/mngr/buddy.hpp"

using namespace Mem;
using namespace hal;
using namespace data_structures;

class BuddyPmmTest : public TestGroupBase
{
    protected:
    // The test memory region, sized to hold two max-order blocks.
    static constexpr size_t kNumPages = 2 * (1UL << BuddyPmm::kMaxOrder);

    // Memory buffers for the allocators themselves
    alignas(kPageSizeBytes) uint8_t bitmap_buffer_[(kNumPages + 7) / 8];
    alignas(kPageSizeBytes) Mem::PageMeta pmt_buffer_[kNumPages];

    // The allocators under test
    BitmapPmm bpmm_{};
    PageMetaTable pmt_{};
    BuddyPmm buddy_pmm_{};

    size_t GetFreeListCount(u8 order) const
    {
        size_t count           = 0;
        VPtr<PageMeta> current = buddy_pmm_.GetFreelistHead(order);
        while (current != nullptr) {
            count++;
            current = PageMeta::AsBuddy(*current).next;
        }
        return count;
    }

    void VerifyFreeListState(u8 order, size_t expected_count) const
    {
        size_t count = GetFreeListCount(order);
        EXPECT_EQ(expected_count, count, "Freelist for order %u has incorrect block count.", order);
    }

    void Setup_() override
    {
        // Initialize the bitmap to all-free (all zeros).
        memset(bitmap_buffer_, 0, sizeof(bitmap_buffer_));
        data_structures::BitMapView bmv{bitmap_buffer_, kNumPages};
        bpmm_.Init(bmv);

        // Initialize the Page Meta Table.
        memset(pmt_buffer_, 0, sizeof(pmt_buffer_));
        pmt_.Init({pmt_buffer_, kNumPages});

        // Initialize the Buddy Allocator. This will populate the freelists from the bitmap.
        buddy_pmm_.Init(bpmm_, pmt_);
    }
};

// ------------------------------
// Initialization Tests
// ------------------------------

TEST_F(BuddyPmmTest, Initialization_MemoryIsCorrectlyCoalesced)
{
    // After Init, all free pages should form two blocks of the maximum order.
    VerifyFreeListState(BuddyPmm::kMaxOrder, 2);

    // All other lists should be empty.
    for (u8 order = 0; order < BuddyPmm::kMaxOrder; ++order) {
        VerifyFreeListState(order, 0_size);
    }
}

// ------------------------------
// Basic Allocation & Deallocation Tests
// ------------------------------

TEST_F(BuddyPmmTest, AllocAndFree_SinglePage)
{
    auto page_res = buddy_pmm_.Alloc({.order = 0});
    ASSERT_TRUE(page_res.has_value());

    buddy_pmm_.Free(page_res.value());

    // The system should return to its initial state.
    VerifyFreeListState(BuddyPmm::kMaxOrder, 2);
    for (u8 order = 0; order < BuddyPmm::kMaxOrder; ++order) {
        VerifyFreeListState(order, 0_size);
    }
}

TEST_F(BuddyPmmTest, Alloc_MultipleDistinctOrders)
{
    auto p1 = buddy_pmm_.Alloc({.order = 0});  // 1 page
    auto p2 = buddy_pmm_.Alloc({.order = 3});  // 8 pages
    auto p3 = buddy_pmm_.Alloc({.order = 5});  // 32 pages

    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(p3.has_value());

    EXPECT_NEQ(p1.value(), p2.value());
    EXPECT_NEQ(p2.value(), p3.value());

    buddy_pmm_.Free(p1.value());
    buddy_pmm_.Free(p2.value());
    buddy_pmm_.Free(p3.value());
}

// ------------------------------
// Block Splitting Tests
// ------------------------------

TEST_F(BuddyPmmTest, Splitting_AllocSmallBlockFromLarge)
{
    // This allocation will force one of the max-order blocks to split repeatedly.
    auto page_res = buddy_pmm_.Alloc({.order = 0});
    ASSERT_TRUE(page_res.has_value());

    // After alloc(0) from a clean state: {max:2} -> {max:1, max-1:1, ..., 1:1, 0:1}
    // and one page of order 0 is allocated. The other page of order 0 is on the freelist.
    VerifyFreeListState(0, 1);
    for (u8 order = 1; order < BuddyPmm::kMaxOrder; ++order) {
        VerifyFreeListState(order, 1);
    }
    VerifyFreeListState(BuddyPmm::kMaxOrder, 1);  // One max-order block remains untouched
}

// ------------------------------
// Block Coalescing Tests
// ------------------------------

TEST_F(BuddyPmmTest, Coalescing_CascadingMerge)
{
    // Allocate all pages as order 0 to fully fragment memory.
    std::array<PPtr<Page>, kNumPages> pages;
    for (size_t i = 0; i < kNumPages; ++i) {
        auto res = buddy_pmm_.Alloc({.order = 0});
        ASSERT_TRUE(res.has_value());
        pages[i] = res.value();
    }

    // Free all pages in random order to test coalescing with random fragmentation patterns.
    size_t seed      = 123456789;
    auto simple_rand = [&]() {
        seed = seed * 1103515245 + 12345;
        return seed;
    };

    std::array<size_t, kNumPages> indices;
    for (size_t i = 0; i < kNumPages; ++i) {
        indices[i] = i;
    }

    // Fisher-Yates shuffle
    for (size_t i = kNumPages; i > 1; --i) {
        size_t j       = simple_rand() % i;
        size_t temp    = indices[i - 1];
        indices[i - 1] = indices[j];
        indices[j]     = temp;
    }

    // Free pages in the shuffled order
    for (size_t i = 0; i < kNumPages; ++i) {
        buddy_pmm_.Free(pages[indices[i]]);
    }

    // All pages should coalesce back into two max-order blocks.
    VerifyFreeListState(BuddyPmm::kMaxOrder, 2);
    for (u8 order = 0; order < BuddyPmm::kMaxOrder; ++order) {
        VerifyFreeListState(order, 0_size);
    }
}

// ------------------------------
// Fragmentation & Stress Tests
// ------------------------------

TEST_F(BuddyPmmTest, Fragmentation_CheckerboardPattern)
{
    std::array<PPtr<Page>, kNumPages> all_pages;

    // Allocate all pages to ensure both max-order blocks are fully split.
    for (size_t i = 0; i < kNumPages; ++i) {
        auto res = buddy_pmm_.Alloc({.order = 0});
        ASSERT_TRUE(res.has_value(), "Initial full allocation failed at page %zu", i);
        all_pages[i] = res.value();
    }

    // Free every other page to create the checkerboard pattern.
    for (size_t i = 0; i < kNumPages; i += 2) {
        buddy_pmm_.Free(all_pages[i]);
    }

    // Attempting to allocate an order 1 block (2 pages) should fail.
    auto res = buddy_pmm_.Alloc({.order = 1});
    EXPECT_FALSE(res.has_value());

    // Cleanup: Free the remaining allocated pages.
    for (size_t i = 1; i < kNumPages; i += 2) {
        buddy_pmm_.Free(all_pages[i]);
    }

    // The allocator should return to its initial coalesced state.
    VerifyFreeListState(BuddyPmm::kMaxOrder, 2);
}

TEST_F(BuddyPmmTest, HighChurn_RandomizedOperations)
{
    StaticVector<std::tuple<PPtr<Page>, u8>, kNumPages> allocated_blocks;

    const int num_operations     = 10000;
    size_t total_allocated_pages = 0;
    size_t seed                  = 123456789;
    auto simple_rand             = [&]() {
        seed = seed * 1103515245 + 12345;
        return seed;
    };

    for (int i = 0; i < num_operations; ++i) {
        bool should_alloc = allocated_blocks.Size() == 0 || ((simple_rand() >> 16) % 2 == 0);

        // Leave a margin to avoid running out of memory, which would halt the test.
        const size_t margin = 1UL << (BuddyPmm::kMaxOrder > 2 ? BuddyPmm::kMaxOrder - 2 : 0);
        if (should_alloc && total_allocated_pages < kNumPages - margin) {
            u8 order = (simple_rand() >> 16) % 4;
            auto res = buddy_pmm_.Alloc({.order = order});
            if (res) {
                allocated_blocks.PushEmplace(res.value(), order);
                total_allocated_pages += (1 << order);
            }
        } else if (allocated_blocks.Size() > 0) {
            size_t idx_to_free = simple_rand() % allocated_blocks.Size();
            auto block_to_free = allocated_blocks[idx_to_free];

            buddy_pmm_.Free(std::get<0>(block_to_free));
            total_allocated_pages -= (1 << std::get<1>(block_to_free));

            // Swap with last element if not already last.
            if (idx_to_free != allocated_blocks.Size() - 1) {
                auto &last_elem = allocated_blocks[allocated_blocks.Size() - 1];
                allocated_blocks[idx_to_free].~tuple();
                new (&allocated_blocks[idx_to_free])
                    std::tuple<PPtr<Page>, u8>(std::move(last_elem));
            }

            allocated_blocks.Pop();
        }
    }

    // Cleanup: Free any remaining blocks
    while (allocated_blocks.Size() > 0) {
        buddy_pmm_.Free(std::get<0>(allocated_blocks.Pop()));
    }

    // Verify the allocator is back in its initial, fully coalesced state.
    VerifyFreeListState(BuddyPmm::kMaxOrder, 2);
}

// ------------------------------
// Error & Boundary Condition Tests
// ------------------------------

FAIL_TEST_F(BuddyPmmTest, Error_DoubleFreeShouldAssert)
{
    auto page_res = buddy_pmm_.Alloc({.order = 0});
    ASSERT_TRUE(page_res.has_value());
    buddy_pmm_.Free(page_res.value());
    // The second free of the same page should trigger an `R_ASSERT` failure.
    buddy_pmm_.Free(page_res.value());
}

TEST_F(BuddyPmmTest, Error_RequestingTooLargeOrder)
{
    auto res = buddy_pmm_.Alloc({.order = BuddyPmm::kMaxOrder + 1});
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), Mem::MemError::InvalidArgument);
}

TEST_F(BuddyPmmTest, Boundary_AllocateMaximumOrder)
{
    auto res1 = buddy_pmm_.Alloc({.order = BuddyPmm::kMaxOrder});
    ASSERT_TRUE(res1.has_value());

    auto res2 = buddy_pmm_.Alloc({.order = BuddyPmm::kMaxOrder});
    ASSERT_TRUE(res2.has_value());

    size_t pfn1 = PageFrameNumber(res1.value());
    size_t pfn2 = PageFrameNumber(res2.value());

    // Verify that we received two different blocks and they are the correct ones.
    const size_t max_block_size = 1UL << BuddyPmm::kMaxOrder;
    bool correct_allocs =
        (pfn1 == 0 && pfn2 == max_block_size) || (pfn1 == max_block_size && pfn2 == 0);
    EXPECT_TRUE(correct_allocs, "Allocator did not return the two max-order blocks as expected.");

    // No more memory should be available.
    auto res3 = buddy_pmm_.Alloc({.order = 0});
    EXPECT_FALSE(res3.has_value());
}

// ------------------------------
// Compile time tests
// ------------------------------

TEST_F(BuddyPmmTest, BuddyAreaSize_WithOrderZero_ReturnsSinglePageSize)
{
    constexpr size_t size = BuddyPmm::BuddyAreaSize(0);
    static_assert(size == kPageSizeBytes);
    EXPECT_EQ(kPageSizeBytes, size);
}

TEST_F(BuddyPmmTest, BuddyAreaSize_WithPositiveOrder_ReturnsCorrectMultipleOfPageSize)
{
    constexpr u8 order             = 5;
    constexpr size_t size          = BuddyPmm::BuddyAreaSize(order);
    constexpr size_t expected_size = (1 << order) * kPageSizeBytes;
    static_assert(size == expected_size);
    EXPECT_EQ(expected_size, size);
}

TEST_F(BuddyPmmTest, BuddyAreaSize_WithMaxOrder_ReturnsCorrectSize)
{
    constexpr u8 order             = BuddyPmm::kMaxOrder;
    constexpr size_t size          = BuddyPmm::BuddyAreaSize(order);
    constexpr size_t expected_size = (1 << order) * kPageSizeBytes;
    static_assert(size == expected_size);
    EXPECT_EQ(expected_size, size);
}
