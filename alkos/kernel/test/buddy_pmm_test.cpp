#include <test_module/test.hpp>

#include <extensions/array.hpp>
#include <extensions/data_structures/data_structures.hpp>
#include <extensions/span.hpp>
#include <extensions/tuple.hpp>

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
    // A 4MB (1024 pages) test memory region, which is 2^10 pages.
    // The max order of the buddy allocator is 10, so this represents two blocks of order 9.
    static constexpr size_t kNumPages = 1024;
    static constexpr u8 kMaxTestOrder = 9;

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

    // Helper to verify the state of the freelists.
    void VerifyFreeListState(u8 order, size_t expected_count) const
    {
        size_t count = GetFreeListCount(order);
        EXPECT_EQ(expected_count, count, "Freelist for order %u has incorrect block count.", order);
    }

    void Setup_() override
    {
        // 1. Initialize the bitmap to all-free (all zeros).
        memset(bitmap_buffer_, 0, sizeof(bitmap_buffer_));
        data_structures::BitMapView bmv{bitmap_buffer_, kNumPages};
        bpmm_.Init(bmv);

        // 2. Initialize the Page Meta Table.
        memset(pmt_buffer_, 0, sizeof(pmt_buffer_));
        pmt_.Init({pmt_buffer_, kNumPages});

        // 3. Initialize the Buddy Allocator. This will populate the freelists from the bitmap.
        buddy_pmm_.Init(bpmm_, pmt_);
    }
};

// ============================================================================
// Test Cases
// ============================================================================

// --- Category 1: Initialization & Basic State ---

TEST_F(BuddyPmmTest, Initialization_MemoryIsCorrectlyCoalesced)
{
    // After Init, all 1024 free pages should form two blocks of order 9 (512 pages each).
    VerifyFreeListState(kMaxTestOrder, 2);

    // All other lists should be empty.
    for (u8 order = 0; order < kMaxTestOrder; ++order) {
        VerifyFreeListState(order, 0_size);
    }
}

// --- Category 2: Basic Allocation & Deallocation ---

TEST_F(BuddyPmmTest, AllocAndFree_SinglePage)
{
    auto page_res = buddy_pmm_.Alloc({.order = 0});
    ASSERT_TRUE(page_res.has_value());

    // Free the page immediately.
    buddy_pmm_.Free(page_res.value());

    // The system should return to its initial state.
    VerifyFreeListState(kMaxTestOrder, 2);
    for (u8 order = 0; order < kMaxTestOrder; ++order) {
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

    // Ensure pointers are distinct and non-overlapping
    EXPECT_NEQ(p1.value(), p2.value());
    EXPECT_NEQ(p2.value(), p3.value());

    buddy_pmm_.Free(p1.value());
    buddy_pmm_.Free(p2.value());
    buddy_pmm_.Free(p3.value());
}

// --- Category 3: Block Splitting ---

TEST_F(BuddyPmmTest, Splitting_AllocSmallBlockFromLarge)
{
    // This allocation will force one of the order 9 blocks to split repeatedly.
    auto page_res = buddy_pmm_.Alloc({.order = 0});
    ASSERT_TRUE(page_res.has_value());

    // After alloc(0) from a clean state: {9:2} -> {9:1, 8:1, 7:1, ..., 1:1, 0:1}
    // and one page of order 0 is allocated. The other page of order 0 is on the freelist.
    VerifyFreeListState(0, 1);
    for (u8 order = 1; order < kMaxTestOrder; ++order) {
        VerifyFreeListState(order, 1);
    }
    VerifyFreeListState(kMaxTestOrder, 1);  // One order 9 block remains untouched
}

// --- Category 4: Block Coalescing ---

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
    // Simple pseudo-random using a linear congruential generator (LCG) since rand() is unavailable.
    size_t seed      = 123456789;
    auto simple_rand = [&]() {
        seed = seed * 1103515245 + 12345;
        return seed;
    };

    // Create an array of indices and shuffle them using Fisher-Yates algorithm
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

    // All pages should coalesce back into two order 9 blocks.
    VerifyFreeListState(kMaxTestOrder, 2);
    for (u8 order = 0; order < kMaxTestOrder; ++order) {
        VerifyFreeListState(order, 0_size);
    }
}

// --- Category 5: Fragmentation & Stress Scenarios ---
//
TEST_F(BuddyPmmTest, Fragmentation_CheckerboardPattern)
{
    std::array<PPtr<Page>, kNumPages> all_pages;

    // 1. Allocate ALL pages to ensure both max-order blocks are fully split.
    for (size_t i = 0; i < kNumPages; ++i) {
        auto res = buddy_pmm_.Alloc({.order = 0});
        ASSERT_TRUE(res.has_value(), "Initial full allocation failed at page %zu", i);
        all_pages[i] = res.value();
    }

    // 2. Free every other page to create the checkerboard pattern across all memory.
    for (size_t i = 0; i < kNumPages; i += 2) {
        buddy_pmm_.Free(all_pages[i]);
    }

    // 3. Now, attempting to allocate an order 1 block (2 pages) should correctly fail.
    auto res = buddy_pmm_.Alloc({.order = 1});
    EXPECT_FALSE(res.has_value());

    // 4. Cleanup: Free the remaining allocated pages.
    for (size_t i = 1; i < kNumPages; i += 2) {
        buddy_pmm_.Free(all_pages[i]);
    }

    // The allocator should return to its initial coalesced state.
    VerifyFreeListState(kMaxTestOrder, 2);
}

TEST_F(BuddyPmmTest, HighChurn_RandomizedOperations)
{
    // Using StaticVector for safer, cleaner management of allocated blocks.
    StaticVector<std::tuple<PPtr<Page>, u8>, kNumPages> allocated_blocks;

    const int num_operations     = 10000;
    size_t total_allocated_pages = 0;
    // TODO: Move this into test suite as utility
    // Simple pseudo-random using a linear congruential generator (LCG) since rand() is unavailable.
    size_t seed      = 123456789;
    auto simple_rand = [&]() {
        seed = seed * 1103515245 + 12345;
        return seed;
    };

    for (int i = 0; i < num_operations; ++i) {
        bool should_alloc = allocated_blocks.Size() == 0 || ((simple_rand() >> 16) % 2 == 0);

        if (should_alloc && total_allocated_pages < kNumPages - (1 << 5)) {  // Keep a small reserve
            // Allocate a small block of pseudo-random order from 0 to 3
            u8 order = (simple_rand() >> 16) % 4;
            auto res = buddy_pmm_.Alloc({.order = order});
            if (res) {
                allocated_blocks.PushEmplace(res.value(), order);
                total_allocated_pages += (1 << order);
            }
        } else if (allocated_blocks.Size() > 0) {
            // Free a random block using the "swap with last and pop" idiom.
            size_t idx_to_free = simple_rand() % allocated_blocks.Size();
            auto block_to_free = allocated_blocks[idx_to_free];

            // Free the memory.
            buddy_pmm_.Free(std::get<0>(block_to_free));
            total_allocated_pages -= (1 << std::get<1>(block_to_free));

            // Swap the element to be removed with the last element if it's not the last one.
            if (idx_to_free != allocated_blocks.Size() - 1) {
                // std::tuple is not copy-assignable, so we destruct and placement-new construct.
                auto &last_elem = allocated_blocks[allocated_blocks.Size() - 1];
                allocated_blocks[idx_to_free].~tuple();
                new (&allocated_blocks[idx_to_free])
                    std::tuple<PPtr<Page>, u8>(std::move(last_elem));
            }

            // Pop the last element.
            allocated_blocks.Pop();
        }
    }

    // Cleanup: Free any remaining blocks
    while (allocated_blocks.Size() > 0) {
        buddy_pmm_.Free(std::get<0>(allocated_blocks.Pop()));
    }

    // Verify the allocator is back in its initial, fully coalesced state.
    VerifyFreeListState(kMaxTestOrder, 2);
}

// --- Category 6: Error & Boundary Conditions ---

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
    // Request an order larger than the maximum supported.
    auto res = buddy_pmm_.Alloc({.order = kMaxTestOrder + 1});
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), Mem::MemError::InvalidArgument);
}

TEST_F(BuddyPmmTest, Boundary_AllocateMaximumOrder)
{
    auto res1 = buddy_pmm_.Alloc({.order = kMaxTestOrder});
    ASSERT_TRUE(res1.has_value());

    auto res2 = buddy_pmm_.Alloc({.order = kMaxTestOrder});
    ASSERT_TRUE(res2.has_value());

    size_t pfn1 = PageFrameNumber(res1.value());
    size_t pfn2 = PageFrameNumber(res2.value());

    // Verify that we received two different blocks and they are the correct ones.
    bool correct_allocs = (pfn1 == 0 && pfn2 == 512) || (pfn1 == 512 && pfn2 == 0);
    EXPECT_TRUE(correct_allocs, "Allocator did not return the two max-order blocks as expected.");

    // No more memory should be available.
    auto res3 = buddy_pmm_.Alloc({.order = 0});
    EXPECT_FALSE(res3.has_value());
}
