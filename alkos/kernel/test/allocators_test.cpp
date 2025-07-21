#include <extensions/internal/allocators/affix_allocator.hpp>
#include <extensions/internal/allocators/allocator_stats.hpp>
#include <extensions/internal/allocators/fallback_adapter.hpp>
#include <extensions/internal/allocators/freelist_allocator.hpp>
#include <extensions/internal/allocators/pool_allocator.hpp>
#include <extensions/internal/allocators/stack_allocator.hpp>
#include <extensions/internal/allocators/stub_allocator.hpp>
#include <extensions/internal/allocators/threshold_adapter.hpp>
#include <test_module/test.hpp>

class AllocatorsTest : public TestGroupBase
{
};

TEST_F(AllocatorsTest, StackAllocator_BasicAllocation)
{
    StackAllocator<1024> allocator;

    auto block = allocator.Allocate(64);
    R_ASSERT_NEQ(nullptr, block.ptr);
    R_ASSERT_EQ(64, block.size);
    R_ASSERT_TRUE(allocator.Owns(block));

    allocator.Deallocate(block);
}

TEST_F(AllocatorsTest, StackAllocator_MultipleAllocations)
{
    StackAllocator<512> allocator1;
    StackAllocator<512> allocator2;

    auto block1 = allocator1.Allocate(100);
    auto block2 = allocator1.Allocate(200);
    auto block3 = allocator2.Allocate(300);

    R_ASSERT_NEQ(nullptr, block1.ptr);
    R_ASSERT_NEQ(nullptr, block2.ptr);
    R_ASSERT_NEQ(nullptr, block3.ptr);

    R_ASSERT_TRUE(allocator1.Owns(block1));
    R_ASSERT_TRUE(allocator1.Owns(block2));
    R_ASSERT_FALSE(allocator1.Owns(block3));

    R_ASSERT_FALSE(allocator2.Owns(block1));
    R_ASSERT_FALSE(allocator2.Owns(block2));
    R_ASSERT_TRUE(allocator2.Owns(block3));

    allocator2.Deallocate(block3);
    allocator1.Deallocate(block2);
    allocator1.Deallocate(block1);
}

TEST_F(AllocatorsTest, StackAllocator_OutOfMemory)
{
    StackAllocator<100> allocator;

    auto block = allocator.Allocate(200);
    R_ASSERT_EQ(nullptr, block.ptr);
    R_ASSERT_EQ(0, block.size);
}

TEST_F(AllocatorsTest, StackAllocator_PositionManagement)
{
    StackAllocator<128> allocator;

    R_ASSERT_EQ(0, allocator.GetCurrentPosition());

    auto block1 = allocator.Allocate(64);
    R_ASSERT_GT(allocator.GetCurrentPosition(), 0);

    allocator.DeallocateAll();
    R_ASSERT_EQ(0, allocator.GetCurrentPosition());

    block1      = allocator.Allocate(64);
    auto pos    = allocator.GetCurrentPosition();
    auto block2 = allocator.Allocate(32);
    allocator.SetPosition(pos);
    block1 = allocator.Allocate(16);
    block1 = allocator.Allocate(32);
    R_ASSERT_EQ(block2.ptr + 16, block1.ptr);

    allocator.DeallocateAll();
}

FAIL_TEST_F(AllocatorsTest, StackAllocator_DeallocateNonTopBlock)
{
    StackAllocator<128> allocator;

    auto block1 = allocator.Allocate(64);
    auto block2 = allocator.Allocate(32);

    allocator.Deallocate(block1);
}

FAIL_TEST_F(AllocatorsTest, StackAllocator_LeakDetection)
{
    StackAllocator<128> allocator;

    auto block1 = allocator.Allocate(64);
    auto block2 = allocator.Allocate(32);
}

TEST_F(AllocatorsTest, PoolAllocator_BasicAllocation)
{
    PoolAllocator<64, 10> allocator;

    auto block = allocator.Allocate(32);
    R_ASSERT_NEQ(nullptr, block.ptr);
    R_ASSERT_EQ(64, block.size);
    R_ASSERT_TRUE(allocator.Owns(block));

    allocator.Deallocate(block);
}

TEST_F(AllocatorsTest, PoolAllocator_ExhaustPool)
{
    PoolAllocator<32, 2> allocator;

    auto block1 = allocator.Allocate(16);
    auto block2 = allocator.Allocate(16);
    auto block3 = allocator.Allocate(16);

    R_ASSERT_NEQ(nullptr, block1.ptr);
    R_ASSERT_NEQ(nullptr, block2.ptr);
    R_ASSERT_EQ(nullptr, block3.ptr);

    allocator.Deallocate(block1);

    auto block4 = allocator.Allocate(16);
    R_ASSERT_NEQ(nullptr, block4.ptr);

    allocator.Deallocate(block2);
    allocator.Deallocate(block4);
}

FAIL_TEST_F(AllocatorsTest, PoolAllocator_LeakDetection)
{
    PoolAllocator<32, 10> allocator;

    auto block1 = allocator.Allocate(16);
    auto block2 = allocator.Allocate(16);

    allocator.Deallocate(block2);
}

TEST_F(AllocatorsTest, FreeListAllocator_BasicAllocation)
{
    FreeListAllocator<StackAllocator<128>, 16, 64, 10> allocator;

    auto block = allocator.Allocate(32);
    R_ASSERT_NEQ(nullptr, block.ptr);
    R_ASSERT_TRUE(allocator.Owns(block));

    allocator.Deallocate(block);
}

TEST_F(AllocatorsTest, FreeListAllocator_ReuseBlocks)
{
    FreeListAllocator<StackAllocator<128>, 16, 64, 10> allocator;

    auto block1 = allocator.Allocate(32);
    R_ASSERT_EQ(64, block1.size);
    auto block2 = allocator.Allocate(32);
    allocator.Deallocate(block1);

    auto block3 = allocator.Allocate(32);
    R_ASSERT_EQ(block1.ptr, block3.ptr);

    allocator.Deallocate(block2);
    allocator.Deallocate(block3);

    // Allocation larger than max block size goes to StackAllocator
    block1 = allocator.Allocate(96);
    allocator.Deallocate(block1);
}

TEST_F(AllocatorsTest, FallbackAdapter_PrimarySuccess)
{
    FallbackAdapter<StackAllocator<128>, PoolAllocator<32, 10>> allocator;

    auto block = allocator.Allocate(64);
    R_ASSERT_NEQ(nullptr, block.ptr);
    R_ASSERT_TRUE(allocator.Owns(block));

    allocator.Deallocate(block);
}

TEST_F(AllocatorsTest, FallbackAdapter_FallbackOnFailure)
{
    FallbackAdapter<StackAllocator<16>, PoolAllocator<32, 10>> allocator;

    auto block1 = allocator.Allocate(16);
    R_ASSERT_NEQ(nullptr, block1.ptr);
    R_ASSERT_TRUE(allocator.Owns(block1));

    auto block2 = allocator.Allocate(24);
    R_ASSERT_NEQ(nullptr, block2.ptr);
    R_ASSERT_EQ(32, block2.size);
    R_ASSERT_TRUE(allocator.Owns(block2));

    auto block3 = allocator.Allocate(64);
    R_ASSERT_EQ(nullptr, block3.ptr);

    allocator.Deallocate(block1);
    allocator.Deallocate(block2);
}

TEST_F(AllocatorsTest, ThresholdAdapter_SmallAllocation)
{
    ThresholdAdapter<128, PoolAllocator<32, 10>, StackAllocator<512>> allocator;

    auto block = allocator.Allocate(64);
    R_ASSERT_EQ(nullptr, block.ptr);
    R_ASSERT_FALSE(allocator.Owns(block));
}

TEST_F(AllocatorsTest, ThresholdAdapter_LargeAllocation)
{
    ThresholdAdapter<128, PoolAllocator<32, 10>, StackAllocator<512>> allocator;

    auto block = allocator.Allocate(256);
    R_ASSERT_NEQ(nullptr, block.ptr);
    R_ASSERT_TRUE(allocator.Owns(block));

    allocator.Deallocate(block);
}

TEST_F(AllocatorsTest, AffixAllocator_WithPrefix)
{
    struct Prefix {
        int value = 42;
    };
    AffixAllocator<StackAllocator<512>, Prefix> allocator;

    auto block = allocator.Allocate(64);
    R_ASSERT_NEQ(nullptr, block.ptr);
    R_ASSERT_TRUE(allocator.Owns(block));

    auto *prefix = reinterpret_cast<Prefix *>(static_cast<byte *>(block.ptr) - sizeof(Prefix));
    R_ASSERT_EQ(42, prefix->value);

    allocator.Deallocate(block);
}

TEST_F(AllocatorsTest, AffixAllocator_WithPrefixAndSuffix)
{
    struct Prefix {
        int value = 42;
    };
    struct Suffix {
        char marker = 'X';
    };
    AffixAllocator<StackAllocator<512>, Prefix, Suffix> allocator;

    auto block = allocator.Allocate(64);
    R_ASSERT_NEQ(nullptr, block.ptr);
    R_ASSERT_TRUE(allocator.Owns(block));

    auto *prefix = reinterpret_cast<Prefix *>(static_cast<byte *>(block.ptr) - sizeof(Prefix));
    auto *suffix = reinterpret_cast<Suffix *>(static_cast<byte *>(block.ptr) + block.size);
    R_ASSERT_EQ(42, prefix->value);
    R_ASSERT_EQ('X', suffix->marker);

    allocator.Deallocate(block);
}

TEST_F(AllocatorsTest, AllocatorStats_GlobalStats)
{
    using enum flags::AllocatorStats;
    AllocatorStats<StackAllocator<512>, Allocations | Deallocations | DebugInfo> allocator;

    auto block1 = allocator.Allocate(64);
    auto block2 = allocator.Allocate(128);

    R_ASSERT_NEQ(nullptr, block1.ptr);
    R_ASSERT_NEQ(nullptr, block2.ptr);

    auto &global_stats1 = allocator.GetGlobalStats();
    R_ASSERT_EQ(2, global_stats1.allocations);
    R_ASSERT_EQ(192, global_stats1.bytes_allocated);
    R_ASSERT_EQ(0, global_stats1.deallocations);
    R_ASSERT_EQ(0, global_stats1.bytes_deallocated);

    allocator.Deallocate(block2);
    allocator.Deallocate(block1);

    auto &global_stats2 = allocator.GetGlobalStats();
    R_ASSERT_EQ(2, global_stats2.deallocations);
    R_ASSERT_EQ(192, global_stats2.bytes_deallocated);
}

TEST_F(AllocatorsTest, AllocatorStats_PerAllocationInfo)
{
    using enum flags::AllocatorStats;
    AllocatorStats<StackAllocator<512>, Allocations | Deallocations | DebugInfo> allocator;

    // clang-format off
    auto line_num1 = __LINE__; auto block1 = allocator.Allocate(64);
    // clang-format on

    auto &stats1 = allocator.GetAllocationInfo(block1);
    R_ASSERT_EQ(block1.ptr, stats1.ptr);
    R_ASSERT_EQ(64, stats1.size);
    R_ASSERT_STREQ(__FILE__, stats1.loc.file_name());
    R_ASSERT_EQ(line_num1, stats1.loc.line());

    allocator.Deallocate(block1);
}

TEST_F(AllocatorsTest, AllocatorStats_AllocationsList)
{
    using enum flags::AllocatorStats;
    AllocatorStats<StackAllocator<512>, Allocations | Deallocations | DebugInfo> allocator;

    // clang-format off
    auto line_num1 = __LINE__; auto block1 = allocator.Allocate(64);
    auto line_num2 = __LINE__; auto block2 = allocator.Allocate(128);
    // clang-format on

    auto *head = allocator.GetAllocations();
    R_ASSERT_NEQ(nullptr, head);
    R_ASSERT_EQ(block2.ptr, head->ptr);
    R_ASSERT_EQ(line_num2, head->loc.line());
    head = head->next;
    R_ASSERT_NEQ(nullptr, head);
    R_ASSERT_EQ(block1.ptr, head->ptr);
    R_ASSERT_EQ(line_num1, head->loc.line());

    allocator.Deallocate(block2);
    allocator.Deallocate(block1);

    head = allocator.GetAllocations();
    R_ASSERT_EQ(nullptr, head);
}

TEST_F(AllocatorsTest, AllocatorStats_DifferentFlags)
{
    using enum flags::AllocatorStats;

    AllocatorStats<StubAllocator<>> allocator1;
    R_ASSERT_EQ(1, sizeof(allocator1));

    AllocatorStats<StubAllocator<>, Allocations> allocator2;
    R_ASSERT_EQ(16, sizeof(allocator2));

    AllocatorStats<StubAllocator<>, Deallocations> allocator3;
    R_ASSERT_EQ(16, sizeof(allocator3));

    AllocatorStats<StubAllocator<>, Allocations | Deallocations> allocator4;
    R_ASSERT_EQ(32, sizeof(allocator4));

    AllocatorStats<StubAllocator<>, DebugInfo> allocator5;
    R_ASSERT_EQ(8, sizeof(allocator5));

    AllocatorStats<StubAllocator<>, Allocations | DebugInfo> allocator6;
    R_ASSERT_EQ(24, sizeof(allocator6));

    AllocatorStats<StubAllocator<>, Allocations | Deallocations | DebugInfo> allocator7;
    R_ASSERT_EQ(40, sizeof(allocator7));
}

TEST_F(AllocatorsTest, ObjectCreation)
{
    int v = 0;
    struct TestObject {
        int *value = nullptr;
        explicit TestObject(int *v) : value(v) { *v = 42; }
        ~TestObject() { *value = 0; }
    };

    StackAllocator<128, 8> allocator;
    auto block = allocator.Allocate(sizeof(TestObject));
    R_ASSERT_NEQ(nullptr, block.ptr);

    auto *obj = Construct<TestObject>(block, &v);
    R_ASSERT_EQ(42, v);

    Destroy<TestObject>(block);
    R_ASSERT_EQ(0, v);

    allocator.Deallocate(block);
}
