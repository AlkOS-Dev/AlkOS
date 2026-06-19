// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <data_structures/lru_cache.hpp>
#include <test_module/test.hpp>

class LruCacheTest : public TestGroupBase
{
    public:
    data_structures::LruCache<int, int, 3> cache;
};

TEST_F(LruCacheTest, InsertionAndLookup)
{
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);

    EXPECT_EQ(3_size, cache.Size());
    EXPECT_EQ(30, *cache.Get(3));
    EXPECT_EQ(20, *cache.Get(2));
    EXPECT_EQ(10, *cache.Get(1));
    EXPECT_EQ(nullptr, cache.Get(42));
}

TEST_F(LruCacheTest, UpdateKeepsSizeAndMovesToFront)
{
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);

    cache.Put(2, 200);
    EXPECT_EQ(3_size, cache.Size());
    EXPECT_EQ(200, *cache.Get(2));

    cache.Put(4, 40);
    EXPECT_EQ(nullptr, cache.Get(1));
    EXPECT_NOT_NULL(cache.Get(4));
    EXPECT_NOT_NULL(cache.Get(2));
    EXPECT_NOT_NULL(cache.Get(3));
}

TEST_F(LruCacheTest, EvictionOfLruEntry)
{
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    EXPECT_NOT_NULL(cache.Get(1));

    cache.Put(4, 30);
    EXPECT_EQ(nullptr, cache.Get(2));
    EXPECT_NOT_NULL(cache.Get(1));
    EXPECT_NOT_NULL(cache.Get(3));

    cache.Get(1);
    cache.Put(5, 40);
    EXPECT_EQ(nullptr, cache.Get(4));
    EXPECT_NOT_NULL(cache.Get(3));
    EXPECT_NOT_NULL(cache.Get(1));
    EXPECT_NOT_NULL(cache.Get(5));
    EXPECT_EQ(3_size, cache.Size());
}

TEST_F(LruCacheTest, EraseAndClear)
{
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);

    EXPECT_EQ(3_size, cache.Size());
    EXPECT_TRUE(cache.Erase(2));
    EXPECT_EQ(nullptr, cache.Get(2));
    EXPECT_EQ(2_size, cache.Size());

    EXPECT_FALSE(cache.Erase(42));

    cache.Clear();
    EXPECT_EQ(0_size, cache.Size());
    EXPECT_TRUE(cache.Empty());
    EXPECT_EQ(nullptr, cache.Get(1));
}
