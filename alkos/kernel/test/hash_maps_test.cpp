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

class RegistryTest : public TestGroupBase
{
};

// ------------------------------
// Hashmap Insert Tests
// ------------------------------

TEST_F(HashmapTest, SingleInsert)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    EXPECT_EQ(0_size, map.Size());

    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(1_size, map.Size());
}

TEST_F(HashmapTest, DuplicateInsert)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(false, map.Insert(1u, 200));
    EXPECT_NOT_NULL(map.Find(1u));
    EXPECT_EQ(100, *map.Find(1u));
    EXPECT_EQ(1_size, map.Size());
}

TEST_F(HashmapTest, OverwriteInsert)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(false, map.Insert<true>(1u, 200));
    EXPECT_EQ(1_size, map.Size());

    int *value = map.Find(1u);
    ASSERT_NOT_NULL(value);
    EXPECT_EQ(200, *value);
}

TEST_F(HashmapTest, MultipleInserts)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    EXPECT_EQ(true, map.Insert(1u, 100));
    EXPECT_EQ(true, map.Insert(2u, 200));
    EXPECT_EQ(true, map.Insert(3u, 300));
    EXPECT_EQ(3_size, map.Size());
}

// ------------------------------
// Hashmap Emplace Tests
// ------------------------------

TEST_F(HashmapTest, EmplaceSingle)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    EXPECT_EQ(true, map.Emplace(1u, 100));
    EXPECT_EQ(1_size, map.Size());

    int *value = map.Find(1u);
    ASSERT_NOT_NULL(value);
    EXPECT_EQ(100, *value);
}

// Test with a more complex type for emplace
struct TestStruct {
    int a;
    int b;
    TestStruct(int x, int y) : a(x), b(y) {}
};

TEST_F(HashmapTest, EmplaceComplexType)
{
    FastMinimalStaticHashmap<u32, TestStruct, 4> map;
    EXPECT_EQ(true, map.Emplace(1u, 10, 20));
    EXPECT_EQ(1_size, map.Size());

    TestStruct *value = map.Find(1u);
    ASSERT_NOT_NULL(value);
    EXPECT_EQ(10, value->a);
    EXPECT_EQ(20, value->b);
}

// ------------------------------
// Hashmap Find Tests
// ------------------------------

TEST_F(HashmapTest, FindNonExisting)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    map.Insert(1u, 100);

    int *value = map.Find(999u);
    EXPECT_EQ(nullptr, value);
}

TEST_F(HashmapTest, FindInEmptyMap)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    int *value = map.Find(1u);
    EXPECT_EQ(nullptr, value);
}

TEST_F(HashmapTest, FindAfterMultipleInserts)
{
    FastMinimalStaticHashmap<u32, int, 8> map;
    map.Insert(1u, 100);
    map.Insert(2u, 200);
    map.Insert(3u, 300);
    map.Insert(4u, 400);

    EXPECT_EQ(100, *map.Find(1u));
    EXPECT_EQ(200, *map.Find(2u));
    EXPECT_EQ(300, *map.Find(3u));
    EXPECT_EQ(400, *map.Find(4u));
    EXPECT_EQ(nullptr, map.Find(5u));
}

// ------------------------------
// Hashmap HasKey Tests
// ------------------------------

TEST_F(HashmapTest, HasKeyExisting)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    map.Insert(1u, 100);

    EXPECT_EQ(true, map.HasKey(1u));
}

TEST_F(HashmapTest, HasKeyNonExisting)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    map.Insert(1u, 100);

    EXPECT_EQ(false, map.HasKey(999u));
}

TEST_F(HashmapTest, HasKeyEmptyMap)
{
    FastMinimalStaticHashmap<u32, int, 4> map;

    EXPECT_EQ(false, map.HasKey(1u));
}

// ------------------------------
// Hashmap Remove Tests
// ------------------------------

TEST_F(HashmapTest, RemoveExisting)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    map.Insert(1u, 100);
    EXPECT_EQ(1_size, map.Size());

    EXPECT_EQ(true, map.Remove(1u));
    EXPECT_EQ(0_size, map.Size());

    int *value = map.Find(1u);
    EXPECT_EQ(nullptr, value);
}

TEST_F(HashmapTest, RemoveNonExisting)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    map.Insert(1u, 100);

    EXPECT_EQ(false, map.Remove(999u));
    EXPECT_EQ(1_size, map.Size());
}

TEST_F(HashmapTest, RemoveFromEmptyMap)
{
    FastMinimalStaticHashmap<u32, int, 4> map;
    EXPECT_EQ(false, map.Remove(1u));
    EXPECT_EQ(0_size, map.Size());
}

TEST_F(HashmapTest, RemoveMiddleElement)
{
    FastMinimalStaticHashmap<u32, int, 8> map;
    map.Insert(1u, 100);
    map.Insert(2u, 200);
    map.Insert(3u, 300);

    EXPECT_EQ(true, map.Remove(2u));
    EXPECT_EQ(2_size, map.Size());

    EXPECT_NOT_NULL(map.Find(1u));
    EXPECT_NULL(map.Find(2u));
    EXPECT_NOT_NULL(map.Find(3u));

    EXPECT_EQ(100, *map.Find(1u));
    EXPECT_EQ(300, *map.Find(3u));
}

// ------------------------------
// Hashmap Hash Function Tests
// ------------------------------

TEST_F(HashmapTest, HashKeyConsistency)
{
    FastMinimalStaticHashmap<u32, int, 4> map;

    size_t hash1 = map.HashKey(42u);
    size_t hash2 = map.HashKey(42u);

    EXPECT_EQ(hash1, hash2);
}

TEST_F(HashmapTest, HashKeyDifferentValues)
{
    FastMinimalStaticHashmap<u32, int, 16> map;

    size_t hash1 = map.HashKey(1u);
    size_t hash2 = map.HashKey(2u);
    size_t hash3 = map.HashKey(1000u);

    // Hashes should be different (though collisions are possible)
    EXPECT_TRUE(hash1 != hash2 || hash2 != hash3 || hash1 != hash3);
}

// ------------------------------
// Hashmap Edge Case Tests
// ------------------------------

TEST_F(HashmapTest, MaxCapacity)
{
    FastMinimalStaticHashmap<u32, int, 2> map;
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
    for (u32 i = 1; i < 1000; ++i) {
        if (map.HashKey(i) == map.HashKey(1u)) {
            stack.Push(i);

            if (stack.Size() == 4) {
                break;  // Found 4 keys with the same hash
            }
        }
    }
    ASSERT_EQ(4_size, stack.Size());

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
}

// ------------------------------
// Registry Tests
// ------------------------------

struct TestEntry : public RegistryEntry {
    int value;
    TestEntry() = default;
    TestEntry(u64 id_val, int val) : value(val) { id = id_val; }
};

TEST_F(RegistryTest, RegisterSingleEntry)
{
    Registry<TestEntry, 4> registry;
    TestEntry entry{1u, 100};

    registry.Register(entry);

    EXPECT_EQ(true, registry.HasKey(1u));
    EXPECT_EQ(100, registry[1u].value);
    EXPECT_EQ(1u, registry[1u].id);
}

TEST_F(RegistryTest, RegisterEmplace)
{
    Registry<TestEntry, 4> registry;

    registry.RegisterEmplace(1u, 1u, 100);

    EXPECT_EQ(true, registry.HasKey(1u));
    EXPECT_EQ(100, registry[1u].value);
    EXPECT_EQ(1u, registry[1u].id);
}

TEST_F(RegistryTest, HasKeyNonExisting)
{
    Registry<TestEntry, 4> registry;

    EXPECT_EQ(false, registry.HasKey(1u));
    registry.RegisterEmplace(1u, 1u, 100);
    EXPECT_EQ(false, registry.HasKey(999u));
}

TEST_F(RegistryTest, SwitchSelectedEntry)
{
    Registry<TestEntry, 4> registry;
    registry.RegisterEmplace(1u, 1u, 100);
    registry.RegisterEmplace(2u, 2u, 200);

    EXPECT_EQ(false, registry.IsSelectedPicked());

    registry.SwitchSelected(1u);

    EXPECT_EQ(true, registry.IsSelectedPicked());
    EXPECT_EQ(100, registry.GetSelected().value);
    EXPECT_EQ(1u, registry.GetSelected().id);
}

TEST_F(RegistryTest, ChangeActiveEntry)
{
    Registry<TestEntry, 4> registry;
    registry.RegisterEmplace(1u, 1u, 100);
    registry.RegisterEmplace(2u, 2u, 200);

    registry.SwitchSelected(1u);
    EXPECT_EQ(100, registry.GetSelected().value);

    registry.SwitchSelected(2u);
    EXPECT_EQ(200, registry.GetSelected().value);
    EXPECT_EQ(2u, registry.GetSelected().id);
}

TEST_F(RegistryTest, AccessOperator)
{
    Registry<TestEntry, 4> registry;
    registry.RegisterEmplace(1u, 1u, 100);

    // Test non-const access
    registry[1u].value = 150;
    EXPECT_EQ(150, registry[1u].value);

    // Test const access
    const auto &const_registry = registry;
    EXPECT_EQ(150, const_registry[1u].value);
}

TEST_F(RegistryTest, IteratorAccess)
{
    Registry<TestEntry, 4> registry;
    registry.RegisterEmplace(1u, 1u, 100);
    registry.RegisterEmplace(2u, 2u, 200);
    registry.RegisterEmplace(3u, 3u, 300);

    size_t count = 0;
    for (const u64 *it = registry.cbegin(); it != registry.cend(); ++it) {
        count++;
        EXPECT_TRUE(registry.HasKey(*it));
    }

    count = 0;
    for (u64 x : registry) {
        count++;
        EXPECT_TRUE(registry.HasKey(x));
    }

    EXPECT_EQ(3_size, count);
}
