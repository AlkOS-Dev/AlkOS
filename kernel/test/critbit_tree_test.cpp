// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <test_module/test.hpp>

#include <data_structures/critbit_tree.hpp>
#include <optional.hpp>
#include <trace_framework.hpp>

using namespace data_structures;

class CritBitTreeTest : public TestGroupBase
{
};

TEST_F(CritBitTreeTest, InsertAndContains)
{
    CritBitTree<int> tree;

    R_ASSERT_FALSE(tree.Contains("key1"));
    R_ASSERT_TRUE(tree.Insert("key1", 42));
    R_ASSERT_TRUE(tree.Contains("key1"));

    R_ASSERT_FALSE(tree.Contains("key2"));
    R_ASSERT_TRUE(tree.Insert("key2", 67));
    R_ASSERT_TRUE(tree.Contains("key2"));
}

TEST_F(CritBitTreeTest, InsertOverwrite)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("key", 42));
    R_ASSERT_TRUE(tree.Contains("key"));

    R_ASSERT_FALSE(tree.Insert<false>("key", 67));
    R_ASSERT_TRUE(tree.Contains("key"));

    R_ASSERT_TRUE(tree.Insert<true>("key", 67));
    R_ASSERT_TRUE(tree.Contains("key"));
}

TEST_F(CritBitTreeTest, Get)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("key", 42));

    auto retrieved = tree.Get("key");
    R_ASSERT_TRUE(retrieved);
    R_ASSERT_EQ(42, **retrieved);

    retrieved = tree.Get("nonexistent");
    R_ASSERT_FALSE(retrieved.has_value());
}

TEST_F(CritBitTreeTest, Remove)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("key", 42));
    R_ASSERT_TRUE(tree.Contains("key"));

    R_ASSERT_TRUE(tree.Remove("key"));
    R_ASSERT_FALSE(tree.Contains("key"));

    R_ASSERT_FALSE(tree.Remove("key"));
}

TEST_F(CritBitTreeTest, ComplexOperations)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("apple", 1));
    R_ASSERT_TRUE(tree.Insert("banana", 2));
    R_ASSERT_TRUE(tree.Insert("apricot", 3));
    R_ASSERT_TRUE(tree.Insert("blueberry", 4));

    R_ASSERT_TRUE(tree.Contains("apple"));
    R_ASSERT_TRUE(tree.Contains("banana"));
    R_ASSERT_TRUE(tree.Contains("apricot"));
    R_ASSERT_TRUE(tree.Contains("blueberry"));

    R_ASSERT_TRUE(tree.Remove("banana"));
    R_ASSERT_FALSE(tree.Contains("banana"));

    auto retrieved = tree.Get("apricot");
    R_ASSERT_TRUE(retrieved);
    R_ASSERT_EQ(3, **retrieved);
}

TEST_F(CritBitTreeTest, EmptyTreeOperations)
{
    CritBitTree<int> tree;

    R_ASSERT_FALSE(tree.Contains("key"));
    R_ASSERT_FALSE(tree.Remove("key"));

    auto retrieved = tree.Get("key");
    R_ASSERT_FALSE(retrieved.has_value());
}

TEST_F(CritBitTreeTest, InsertSimilarKeys)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("test", 10));
    R_ASSERT_TRUE(tree.Insert("testing", 20));
    R_ASSERT_TRUE(tree.Insert("tester", 30));

    R_ASSERT_TRUE(tree.Contains("test"));
    R_ASSERT_TRUE(tree.Contains("testing"));
    R_ASSERT_TRUE(tree.Contains("tester"));

    auto retrieved = tree.Get("testing");
    R_ASSERT_TRUE(retrieved);
    R_ASSERT_EQ(20, **retrieved);
}

TEST_F(CritBitTreeTest, RemoveNonExistentKey)
{
    CritBitTree<int> tree;

    R_ASSERT_FALSE(tree.Remove("nonexistent"));

    R_ASSERT_TRUE(tree.Insert("existent", 100));
    R_ASSERT_TRUE(tree.Remove("existent"));
    R_ASSERT_FALSE(tree.Remove("existent"));
}

TEST_F(CritBitTreeTest, ConstGet)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("constKey", 55));

    const auto &const_tree = tree;
    auto retrieved         = const_tree.Get("constKey");
    R_ASSERT_TRUE(retrieved);
    R_ASSERT_EQ(55, **retrieved);
}

TEST_F(CritBitTreeTest, IndexOperator)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("indexKey", 77));

    auto retrieved = tree["indexKey"];
    R_ASSERT_TRUE(retrieved);
    R_ASSERT_EQ(77, **retrieved);

    retrieved = tree["nonexistentKey"];
    R_ASSERT_FALSE(retrieved.has_value());
}

TEST_F(CritBitTreeTest, ComplexFilePaths)
{
    CritBitTree<int> tree;

    R_ASSERT_TRUE(tree.Insert("/home/user/docs", 1));
    R_ASSERT_TRUE(tree.Insert("/home/user/images", 2));
    R_ASSERT_TRUE(tree.Insert("/var/log/syslog", 3));
    R_ASSERT_TRUE(tree.Insert("/etc/config/settings", 4));

    R_ASSERT_TRUE(tree.Contains("/home/user/docs"));
    R_ASSERT_TRUE(tree.Contains("/home/user/images"));
    R_ASSERT_TRUE(tree.Contains("/var/log/syslog"));
    R_ASSERT_TRUE(tree.Contains("/etc/config/settings"));

    auto retrieved = tree.Get("/var/log/syslog");
    R_ASSERT_TRUE(retrieved);
    R_ASSERT_EQ(3, **retrieved);
}
