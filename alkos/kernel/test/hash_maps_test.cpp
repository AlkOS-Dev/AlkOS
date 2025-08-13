/* internal includes */
#include <assert.h>
#include <string.h>
#include <extensions/data_structures/array_structures.hpp>
#include <extensions/data_structures/hash_maps.hpp>
#include <test_module/test.hpp>

using namespace data_structures;

class HashmapTest : public TestGroupBase
{
};

// ------------------------------
// Insert Tests
// ------------------------------

TEST_F(HashmapTest, SingleInsert)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    EXPECT_EQ(0_size, map.Size());

    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(1_size, map.Size());
}

TEST_F(HashmapTest, DuplicateInsert)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(false, map.Insert(1u, 200));
    EXPECT_NOT_NULL(map.Find(1u));
    EXPECT_EQ(100, *map.Find(1u));
    EXPECT_EQ(1_size, map.Size());
}

TEST_F(HashmapTest, OverwriteInsert)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(false, map.Insert<true>(1u, 200));
    EXPECT_EQ(1_size, map.Size());

    int* value = map.Find(1u);
    ASSERT_NOT_NULL(value);
    EXPECT_EQ(200, *value);
}

TEST_F(HashmapTest, MultipleInserts)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(true, map.Insert(2u, 200));
    EXPECT_EQ(true, map.Insert(3u, 300));
    EXPECT_EQ(3_size, map.Size());
}

// ------------------------------
// Find Tests
// ------------------------------

TEST_F(HashmapTest, FindNonExisting)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    map.Insert(1u, 100);

    int* value = map.Find(999u);
    EXPECT_EQ(nullptr, value);
}

TEST_F(HashmapTest, FindInEmptyMap)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    int* value = map.Find(1u);
    EXPECT_EQ(nullptr, value);
}

// ------------------------------
// Remove Tests
// ------------------------------

TEST_F(HashmapTest, RemoveExisting)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    map.Insert(1u, 100);
    EXPECT_EQ(1_size, map.Size());

    EXPECT_EQ(true, map.Remove(1u));
    EXPECT_EQ(0_size, map.Size());

    int* value = map.Find(1u);
    EXPECT_EQ(nullptr, value);
}

TEST_F(HashmapTest, RemoveNonExisting)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    map.Insert(1u, 100);

    EXPECT_EQ(false, map.Remove(999u));
    EXPECT_EQ(1_size, map.Size());
}

TEST_F(HashmapTest, RemoveFromEmptyMap)
{
    FastMinimalStaticHashmap<uint32_t, int, 4> map;
    EXPECT_EQ(false, map.Remove(1u));
    EXPECT_EQ(0_size, map.Size());
}

// ------------------------------
// Edge Case Tests
// ------------------------------

TEST_F(HashmapTest, MaxCapacity)
{
    FastMinimalStaticHashmap<uint32_t, int, 2> map;
    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(true, map.Insert(2u, 200));
    EXPECT_EQ(2_size, map.Size());
    EXPECT_NOT_NULL(map.Find(1u));
    EXPECT_NOT_NULL(map.Find(2u));
    EXPECT_EQ(100, *map.Find(1u));
    EXPECT_EQ(200, *map.Find(2u));
}

TEST_F(HashmapTest, LinearProbingCollision)
{
    using map_t = FastMinimalStaticHashmap<u32, int, 4>;
    map_t map;

    // Find 4 colliding keys
    StaticVector<u32, 4> stack{};
    for (u32 i = 0; i < 1000; ++i) {
        if (map_t::HashKey(i) == 0) {
            stack.Push(i);

            if (stack.Size() == 4) {
                break;  // Found 4 keys with the same hash
            }
        }
    }

    TRACE_DEBUG("%u, %u, %u, %u", stack[0], stack[1], stack[2], stack[3]);

    ASSERT_TRUE(map.Insert(stack[0], 100));
    ASSERT_TRUE(map.Insert(stack[1], 200));
    ASSERT_TRUE(map.Insert(stack[2], 300));
    ASSERT_TRUE(map.Insert(stack[3], 400));

    ASSERT_NOT_NULL(map.Find(stack[0]));
    ASSERT_NOT_NULL(map.Find(stack[1]));
    ASSERT_NOT_NULL(map.Find(stack[2]));
    ASSERT_NOT_NULL(map.Find(stack[3]));

    EXPECT_EQ(100, *map.Find(stack[0]));
    EXPECT_EQ(200, *map.Find(stack[1]));
    EXPECT_EQ(300, *map.Find(stack[2]));
    EXPECT_EQ(400, *map.Find(stack[3]));

    EXPECT_TRUE(map.Remove(stack[0]));
    EXPECT_NULL(map.Find(stack[0]));
    EXPECT_EQ(3_size, map.Size());

    EXPECT_NOT_NULL(map.Find(stack[1]));
    EXPECT_EQ(200, *map.Find(stack[1]));
    EXPECT_TRUE(map.Remove(stack[1]));
    EXPECT_NULL(map.Find(stack[1]));
    EXPECT_EQ(2_size, map.Size());

    // Previously it was failing here so close attention
    EXPECT_NOT_NULL(map.Find(stack[2]));
    EXPECT_EQ(300, *map.Find(stack[2]));
    EXPECT_NOT_NULL(map.Find(stack[3]));
    EXPECT_EQ(400, *map.Find(stack[3]));

    EXPECT_TRUE(map.Remove(stack[2]));
    EXPECT_NULL(map.Find(stack[2]));
    EXPECT_EQ(1_size, map.Size());

    EXPECT_TRUE(map.Remove(stack[3]));
    EXPECT_NULL(map.Find(stack[3]));
    EXPECT_EQ(0_size, map.Size());
}
