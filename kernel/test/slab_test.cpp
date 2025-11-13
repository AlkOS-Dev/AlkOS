#include <test_module/test.hpp>

#include <string.h>
#include <data_structures/data_structures.hpp>
#include <hal/constants.hpp>
#include <mem/phys/mngr/buddy.hpp>
#include <mem/phys/mngr/slab.hpp>

using namespace Mem;
using namespace hal;
using namespace data_structures;

class SlabTest : public TestGroupBase
{
    protected:
    static constexpr size_t kNumPages = 2 * (1UL << BuddyPmm::kMaxOrder);

    alignas(kPageSizeBytes) uint8_t bitmap_buffer_[(kNumPages + 7) / 8];
    alignas(kPageSizeBytes) Mem::PageMeta pmt_buffer_[kNumPages];

    BitmapPmm bpmm_{};
    PageMetaTable pmt_{};
    BuddyPmm buddy_pmm_{};

    void Setup_() override
    {
        memset(bitmap_buffer_, 0, sizeof(bitmap_buffer_));
        data_structures::BitMapView bmv{bitmap_buffer_, kNumPages};
        bpmm_.Init(bmv);

        memset(pmt_buffer_, 0, sizeof(pmt_buffer_));
        pmt_.Init({pmt_buffer_, kNumPages});

        buddy_pmm_.Init(bpmm_, pmt_);
    }
};

struct TestObject {
    u64 a;
    u32 b;
    u16 c;
    u8 d;
};

TEST_F(SlabTest, Constructor_GivenValidBuddyPmm_Succeeds) { Slab<TestObject, 2> slab(buddy_pmm_); }

FAIL_TEST_F(SlabTest, Constructor_GivenOomBuddyPmm_Panics)
{
    auto res = buddy_pmm_.Alloc({.order = BuddyPmm::kMaxOrder});
    ASSERT_TRUE(res.has_value());
    res = buddy_pmm_.Alloc({.order = BuddyPmm::kMaxOrder});
    ASSERT_TRUE(res.has_value());

    Slab<TestObject, 0> slab(buddy_pmm_);
}

TEST_F(SlabTest, Alloc_OnNewSlab_ReturnsValidPointer)
{
    Slab<TestObject, 1> slab(buddy_pmm_);
    auto alloc_res = slab.Alloc();
    EXPECT_TRUE(alloc_res.has_value());
    EXPECT_NEQ(nullptr, alloc_res.value());
}

TEST_F(SlabTest, Alloc_OnExhaustedSlab_ReturnsOutOfMemoryError)
{
    Slab<TestObject, 0> slab(buddy_pmm_);
    using EfficiencyInfo  = SlabEfficiency::SlabEfficiencyInfo<sizeof(TestObject), 0>;
    const size_t capacity = EfficiencyInfo::kCapacity;

    for (size_t i = 0; i < capacity; ++i) {
        auto res = slab.Alloc();
        ASSERT_TRUE(res.has_value());
    }

    auto res = slab.Alloc();
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(MemError::OutOfMemory, res.error());
}

TEST_F(SlabTest, Alloc_AfterFree_ReturnsReusedPointer)
{
    Slab<TestObject, 1> slab(buddy_pmm_);
    auto ptr1_res = slab.Alloc();
    ASSERT_TRUE(ptr1_res.has_value());
    VPtr<TestObject> ptr1 = ptr1_res.value();

    slab.Free(ptr1);

    auto ptr2_res = slab.Alloc();
    ASSERT_TRUE(ptr2_res.has_value());
    VPtr<TestObject> ptr2 = ptr2_res.value();

    EXPECT_EQ(ptr1, ptr2);
}

TEST_F(SlabTest, Free_ValidPointerFromSlab_Succeeds)
{
    Slab<TestObject, 1> slab(buddy_pmm_);
    auto ptr_res = slab.Alloc();
    ASSERT_TRUE(ptr_res.has_value());
    slab.Free(ptr_res.value());
}

FAIL_TEST_F(SlabTest, Free_PointerFromDifferentSlab_TriggersAssertion)
{
    Slab<TestObject, 1> slab_a(buddy_pmm_);
    Slab<TestObject, 1> slab_b(buddy_pmm_);

    auto ptr_res = slab_a.Alloc();
    ASSERT_TRUE(ptr_res.has_value());

    slab_b.Free(ptr_res.value());
}

FAIL_TEST_F(SlabTest, Free_UnalignedPointer_TriggersAssertion)
{
    Slab<TestObject, 1> slab(buddy_pmm_);
    auto ptr_res = slab.Alloc();
    ASSERT_TRUE(ptr_res.has_value());

    VPtr<u8> byte_ptr              = reinterpret_cast<VPtr<u8>>(ptr_res.value());
    VPtr<TestObject> unaligned_ptr = reinterpret_cast<VPtr<TestObject>>(byte_ptr + 1);

    slab.Free(unaligned_ptr);
}

TEST_F(SlabTest, SlabLifecycle_FullAllocThenFullFree_SlabIsFullyReusable)
{
    Slab<TestObject, 0> slab(buddy_pmm_);
    using EfficiencyInfo  = SlabEfficiency::SlabEfficiencyInfo<sizeof(TestObject), 0>;
    const size_t capacity = EfficiencyInfo::kCapacity;

    StaticVector<VPtr<TestObject>, capacity> pointers;

    for (size_t i = 0; i < capacity; ++i) {
        auto res = slab.Alloc();
        ASSERT_TRUE(res.has_value());
        pointers.Push(res.value());
    }

    ASSERT_FALSE(slab.Alloc().has_value());

    for (size_t i = 0; i < pointers.Size(); ++i) {
        slab.Free(pointers[i]);
    }

    for (size_t i = 0; i < capacity; ++i) {
        auto res = slab.Alloc();
        EXPECT_TRUE(res.has_value());
    }
}

TEST_F(SlabTest, SlabLifecycle_InterleavedAllocAndFree_MaintainsIntegrity)
{
    Slab<TestObject, 0> slab(buddy_pmm_);
    using EfficiencyInfo  = SlabEfficiency::SlabEfficiencyInfo<sizeof(TestObject), 0>;
    const size_t capacity = EfficiencyInfo::kCapacity;

    StaticVector<VPtr<TestObject>, capacity> pointers;

    auto alloc_one = [&]() {
        auto res = slab.Alloc();
        if (res.has_value()) {
            pointers.Push(res.value());
        }
        return res.has_value();
    };

    auto free_one = [&]() {
        if (pointers.Size() > 0) {
            slab.Free(pointers.Pop());
        }
    };

    ASSERT_TRUE(alloc_one());
    ASSERT_TRUE(alloc_one());
    free_one();
    ASSERT_TRUE(alloc_one());
    free_one();
    free_one();

    size_t alloc_count = 0;
    while (slab.Alloc().has_value()) {
        alloc_count++;
    }

    EXPECT_EQ(capacity - 1, alloc_count);
}
