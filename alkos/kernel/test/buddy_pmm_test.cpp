#include <extensions/span.hpp>
#include <extensions/types.hpp>
#include <test_module/test.hpp>

#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/phys/mngr/buddy.hpp"

using namespace Mem;
using namespace hal;

class BuddyPmmTest : public TestGroupBase
{
    protected:
    static constexpr size_t kNumPages    = 1024;
    static constexpr size_t kBitmapBytes = kNumPages / 8;

    u8 bitmap_buffer_[kBitmapBytes];
    Mem::PageMeta pmt_buffer_[kNumPages];

    BitmapPmm bpmm_{};
    PageMetaTable pmt_{};
    BuddyPmm buddy_pmm_{};

    void Setup_() override
    {
        // BitmapPmm Init
        memset(bitmap_buffer_, 0, sizeof(bitmap_buffer_));
        data_structures::BitMapView bmv{bitmap_buffer_, kNumPages};
        bpmm_.Init(bmv);

        // PageMetaTable Init
        memset(pmt_buffer_, 0, sizeof(pmt_buffer_));
        pmt_.Init({pmt_buffer_, kNumPages});

        // BuddyPmm Init
        buddy_pmm_.Init(bpmm_, pmt_);
    }
};

TEST_F(BuddyPmmTest, AllocOrderZero)
{
    auto page = buddy_pmm_.Alloc({.order = 0});
    R_ASSERT_TRUE(page.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(page.value()), 0);

    auto &meta = pmt_.GetPageMeta(0);
    R_ASSERT_EQ(meta.type, Mem::PageMetaType::Allocated);
    R_ASSERT_EQ(meta.order, 0);
}

TEST_F(BuddyPmmTest, AllocHigherOrder)
{
    auto page = buddy_pmm_.Alloc({.order = 3});
    R_ASSERT_TRUE(page.has_value());

    // The buddy allocator should find the first aligned block.
    // The memory is empty, so the first free PFN is 0, which is
    // correctly aligned for any order.
    const size_t expected_pfn = 0;
    R_ASSERT_EQ(Mem::PageFrameNumber(page.value()), expected_pfn);

    auto &meta = pmt_.GetPageMeta(expected_pfn);
    R_ASSERT_EQ(meta.type, Mem::PageMetaType::Allocated);
    R_ASSERT_EQ(meta.order, 3);
}

TEST_F(BuddyPmmTest, Splitting)
{
    // Request a small block, forcing the initial large block to be split.
    auto page = buddy_pmm_.Alloc({.order = 0});
    R_ASSERT_TRUE(page.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(page.value()), 0);

    // The initial block (order 9 at pfn 0) was split.
    // This created buddies of various orders.
    for (uint8_t order = 0; order < 9; ++order) {
        size_t buddy_pfn = 1 << order;
        auto &meta       = pmt_.GetPageMeta(buddy_pfn);
        R_ASSERT_EQ(meta.type, Mem::PageMetaType::Buddy);
        R_ASSERT_EQ(meta.order, order);
    }
}

TEST_F(BuddyPmmTest, Coalescing)
{
    auto page0 = buddy_pmm_.Alloc({.order = 0});
    R_ASSERT_TRUE(page0.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(page0.value()), 0);

    auto page1 = buddy_pmm_.Alloc({.order = 0});
    R_ASSERT_TRUE(page1.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(page1.value()), 1);

    buddy_pmm_.Free(page0.value());
    {
        auto &meta = pmt_.GetPageMeta(0);
        R_ASSERT_EQ(meta.type, Mem::PageMetaType::Buddy);
        R_ASSERT_EQ(meta.order, 0);
    }

    buddy_pmm_.Free(page1.value());
    {
        // Should coalesce into a block of order 1
        auto &meta = pmt_.GetPageMeta(0);
        R_ASSERT_EQ(meta.type, Mem::PageMetaType::Buddy);
        R_ASSERT_EQ(meta.order, 9);
    }
}

TEST_F(BuddyPmmTest, OutOfMemory)
{
    // There are 1024 pages total, forming two blocks of order 9.
    auto p1 = buddy_pmm_.Alloc({.order = 9});
    R_ASSERT_TRUE(p1.has_value());

    auto p2 = buddy_pmm_.Alloc({.order = 9});
    R_ASSERT_TRUE(p2.has_value());

    auto page3 = buddy_pmm_.Alloc({.order = 0});
    R_ASSERT_FALSE(page3.has_value());
    R_ASSERT_EQ(page3.error(), Mem::MemError::OutOfMemory);
}

TEST_F(BuddyPmmTest, ComplexScenario)
{
    auto p1 = buddy_pmm_.Alloc({.order = 2});  // pfn 0, size 4
    auto p2 = buddy_pmm_.Alloc({.order = 2});  // pfn 4, size 4
    auto p3 = buddy_pmm_.Alloc({.order = 3});  // pfn 8, size 8
    auto p4 = buddy_pmm_.Alloc({.order = 2});  // pfn 16, size 4

    R_ASSERT_TRUE(p1.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(p1.value()), 0);
    R_ASSERT_TRUE(p2.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(p2.value()), 4);
    R_ASSERT_TRUE(p3.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(p3.value()), 8);
    R_ASSERT_TRUE(p4.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(p4.value()), 16);

    buddy_pmm_.Free(p1.value());  // frees pfn 0
    buddy_pmm_.Free(p3.value());  // frees pfn 8

    auto p5 = buddy_pmm_.Alloc({.order = 3});  // should reuse p3's slot at pfn 8
    R_ASSERT_TRUE(p5.has_value());
    R_ASSERT_EQ(Mem::PageFrameNumber(p5.value()), 8);

    buddy_pmm_.Free(p2.value());  // frees pfn 4
    // now pfn 0 and pfn 4 are free, should coalesce into order 3 block at 0

    auto &meta = pmt_.GetPageMeta(0);
    R_ASSERT_EQ(meta.type, Mem::PageMetaType::Buddy);
    R_ASSERT_EQ(meta.order, 3);
}
