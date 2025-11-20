== = kernel / test / slab_test.cpp == =
#include "mem/phys/mngr/slab.hpp"
#include <test_module/test.hpp>
#include "modules/memory.hpp"

                                          using namespace Mem;

class SlabTest : public TestGroupBase
{
    protected:
    BuddyPmm &GetGlobalBuddy() { return MemoryModule::Get().GetBuddyPmm(); }
};

// ------------------------------
// KmemCache Init Tests
// ------------------------------

TEST_F(SlabTest, Init_GivenValidParameters_PreparesCacheForAllocation)
{
    KmemCache cache;
    // 32 bytes object, order 0 slab
    // Capacity approx PageSize / 32
    cache.Init(32, 0, 120, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> ptr = cache.Alloc();
    EXPECT_NOT_NULL(ptr);

    // Cleanup not fully possible without FreeAll, leaking slab for test
}

TEST_F(SlabTest, Init_GivenValidOffSlabParameters_PreparesCacheForAllocation)
{
    // Setup a meta cache first (using global buddy)
    KmemCache meta_cache;
    // Meta cache for byte indices
    meta_cache.Init(1, 0, 4000, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    KmemCache cache;
    // Large objects, off-slab
    cache.Init(2048, 0, 2, KmemCache::MetadataSize::Byte, true, &meta_cache, &GetGlobalBuddy());

    VPtr<void> ptr = cache.Alloc();
    EXPECT_NOT_NULL(ptr);
}

FAIL_TEST_F(SlabTest, Init_GivenNullBuddyPmm_Asserts)
{
    KmemCache cache;
    cache.Init(32, 0, 10, KmemCache::MetadataSize::Byte, false, nullptr, nullptr);
}

FAIL_TEST_F(SlabTest, Init_GivenZeroObjectSize_Asserts)
{
    KmemCache cache;
    cache.Init(0, 0, 10, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());
}

FAIL_TEST_F(SlabTest, Init_OffSlabWithNullMetaCache_Asserts)
{
    KmemCache cache;
    cache.Init(32, 0, 10, KmemCache::MetadataSize::Byte, true, nullptr, &GetGlobalBuddy());
}

// ------------------------------
// KmemCache Alloc Tests
// ------------------------------

TEST_F(SlabTest, Alloc_OnEmptyCache_ReturnsNonNullPointer)
{
    KmemCache cache;
    cache.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> ptr = cache.Alloc();
    EXPECT_NOT_NULL(ptr);
}

TEST_F(SlabTest, Alloc_MultipleTimes_ReturnsUniquePointers)
{
    KmemCache cache;
    cache.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> ptr1 = cache.Alloc();
    VPtr<void> ptr2 = cache.Alloc();
    VPtr<void> ptr3 = cache.Alloc();

    EXPECT_NOT_NULL(ptr1);
    EXPECT_NOT_NULL(ptr2);
    EXPECT_NOT_NULL(ptr3);

    EXPECT_NEQ(ptr1, ptr2);
    EXPECT_NEQ(ptr2, ptr3);
    EXPECT_NEQ(ptr1, ptr3);
}

TEST_F(SlabTest, Alloc_WhenBackingAllocatorIsExhausted_ReturnsNullptr)
{
    // TODO: Implement
    // This requires creating a buddy pmm with no free pages which
    // requires initializing the buddy pmm with a bitmap pmm that has no free pages and a
    // PageMetaTable that has no free pages. It also requires an empty bitmap
}

TEST_F(SlabTest, Alloc_ExhaustingInitialSlab_SuccessfullyAllocatesFromNewSlab)
{
    // Create a cache that fits exactly 2 objects per page (order 0 = 4096 bytes)
    // Object size ~2000
    KmemCache cache;
    cache.Init(2000, 0, 2, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> p1 = cache.Alloc();
    VPtr<void> p2 = cache.Alloc();
    EXPECT_NOT_NULL(p1);
    EXPECT_NOT_NULL(p2);

    // One of these should trigger a new slab allocation
    for (int i = 0; i < 5; i++) {
        VPtr<void> p = cache.Alloc();
        EXPECT_NOT_NULL(p);
    }
}

// ------------------------------
// KmemCache Free Tests
// ------------------------------

TEST_F(SlabTest, Free_ValidPointer_AllowsPointerToBeReallocated)
{
    KmemCache cache;
    cache.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> ptr = cache.Alloc();
    cache.Free(ptr);

    // Should not crash and allow alloc
    VPtr<void> ptr2 = cache.Alloc();
    EXPECT_NOT_NULL(ptr2);
}

FAIL_TEST_F(SlabTest, Free_NullPointer_Asserts)
{
    KmemCache cache;
    cache.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());
    cache.Free(nullptr);
}

FAIL_TEST_F(SlabTest, Free_PointerNotFromThisCache_Asserts)
{
    KmemCache cacheA;
    cacheA.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    KmemCache cacheB;
    cacheB.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> ptrB = cacheB.Alloc();
    cacheA.Free(ptrB);
}

FAIL_TEST_F(SlabTest, Free_DoubleFree_Asserts)
{
    KmemCache cache;
    cache.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> ptr = cache.Alloc();
    cache.Free(ptr);
    cache.Free(ptr);
}

TEST_F(SlabTest, AllocAndFreeCycle_MaintainsStableState)
{
    KmemCache cache;
    cache.Init(64, 0, 60, KmemCache::MetadataSize::Byte, false, nullptr, &GetGlobalBuddy());

    VPtr<void> ptrs[10];

    // Fill
    for (int i = 0; i < 10; ++i) ptrs[i] = cache.Alloc();

    // Free half
    for (int i = 0; i < 5; ++i) cache.Free(ptrs[i]);

    // Refill
    for (int i = 0; i < 5; ++i) ptrs[i] = cache.Alloc();

    // Validate all are valid
    for (int i = 0; i < 10; ++i) EXPECT_NOT_NULL(ptrs[i]);
}

// ------------------------------
// SlabAllocator Tests
// ------------------------------

TEST_F(SlabTest, Init_WithValidBuddyPmm_PreparesAllocatorForUse)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    KmemCache *cache = allocator.GetCache(64);
    EXPECT_NOT_NULL(cache);

    VPtr<void> ptr = cache->Alloc();
    EXPECT_NOT_NULL(ptr);
}

TEST_F(SlabTest, GetCache_ForSizeWithinRange_ReturnsNonNullCache)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    EXPECT_NOT_NULL(allocator.GetCache(8));
    EXPECT_NOT_NULL(allocator.GetCache(4096));
}

TEST_F(SlabTest, GetCache_ForSizesInSameBucket_ReturnsSameCacheInstance)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    // 16 byte bucket covers 9..16
    KmemCache *c1 = allocator.GetCache(9);
    KmemCache *c2 = allocator.GetCache(15);

    EXPECT_EQ(c1, c2);
}

TEST_F(SlabTest, GetCache_ForSizesInDifferentBuckets_ReturnsDifferentCacheInstances)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    KmemCache *c1 = allocator.GetCache(16);
    KmemCache *c2 = allocator.GetCache(17);  // Goes to 32 bucket

    EXPECT_NEQ(c1, c2);
}

TEST_F(SlabTest, GetCache_ForZeroSize_ReturnsNullptr)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());
    EXPECT_NULL(allocator.GetCache(0));
}

TEST_F(SlabTest, GetCache_ForSizeLargerThanMax_ReturnsNullptr)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());
    EXPECT_NULL(allocator.GetCache(4097));
}

TEST_F(SlabTest, GetCache_ForSmallestSizeClass_ReturnsCorrectCache)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());
    // 8 is the smallest
    EXPECT_NOT_NULL(allocator.GetCache(1));
    EXPECT_NOT_NULL(allocator.GetCache(8));
    EXPECT_EQ(allocator.GetCache(1), allocator.GetCache(8));
}

TEST_F(SlabTest, GetCache_ForLargestSizeClass_ReturnsCorrectCache)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());
    EXPECT_NOT_NULL(allocator.GetCache(4096));
}

TEST_F(SlabTest, GetCacheFromIndex_ForValidIndex_ReturnsNonNullCache)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    for (size_t i = 0; i < SlabAllocator::kNumSizeClasses; ++i) {
        EXPECT_NOT_NULL(allocator.GetCacheFromIndex(i));
    }
}

TEST_F(SlabTest, GetCacheFromIndex_ForOutOfBoundsIndex_ReturnsNullptr)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    EXPECT_NULL(allocator.GetCacheFromIndex(SlabAllocator::kNumSizeClasses));
}

TEST_F(SlabTest, GetCacheFromIndex_ForZeroIndex_ReturnsFirstCache)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    KmemCache *idx0  = allocator.GetCacheFromIndex(0);
    KmemCache *size8 = allocator.GetCache(8);
    EXPECT_EQ(idx0, size8);
}

TEST_F(SlabTest, GetCacheFromIndex_ForLastValidIndex_ReturnsLastCache)
{
    SlabAllocator allocator;
    allocator.Init(GetGlobalBuddy());

    KmemCache *last    = allocator.GetCacheFromIndex(SlabAllocator::kNumSizeClasses - 1);
    KmemCache *sizeMax = allocator.GetCache(4096);
    EXPECT_EQ(last, sizeMax);
}
