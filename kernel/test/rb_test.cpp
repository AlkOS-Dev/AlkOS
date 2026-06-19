// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <data_structures/linked_list.hpp>
#include <data_structures/maps/intrusive_rb_tree.hpp>
#include <mem/heap.hpp>
#include <random.hpp>
#include <test_module/test.hpp>
#include <trace_framework.hpp>

namespace data_structures::test
{

struct TestNode : IntrusiveRbNode<TestNode, int, 0>, IntrusiveListNode<TestNode, 0> {
    TestNode *tracking_next = nullptr;
    TestNode *tracking_prev = nullptr;

    int payload = 0;

    TestNode(int k, int v) : payload(v)
    {
        this->key    = k;
        this->color  = Color::kBlack;
        this->left   = nullptr;
        this->right  = nullptr;
        this->parent = nullptr;
        this->next   = nullptr;
    }
};

// ------------------------------
// Test Fixture with Manual Memory Management
// ------------------------------

class IntrusiveRBTreeTest : public TestGroupBase
{
    public:
    using TreeType = IntrusiveRBTree<TestNode, int, 0>;

    TreeType tree{};
    TestNode *tracking_head = nullptr;
    size_t allocated_count  = 0;

    void Setup_() override
    {
        tracking_head   = nullptr;
        allocated_count = 0;
    }

    void TearDown_() override
    {
        TestNode *current = tracking_head;
        while (current) {
            TestNode *next = current->tracking_next;
            delete current;
            current = next;
        }
        tracking_head = nullptr;
    }

    TestNode *NewNode(int key, int val = 0)
    {
        auto mem   = Mem::KMalloc(sizeof(TestNode)).value();
        auto *node = new (mem) TestNode(key, val);

        node->tracking_next = tracking_head;
        if (tracking_head) {
            tracking_head->tracking_prev = node;
        }
        tracking_head = node;
        allocated_count++;

        return node;
    }

    int ValidateRBProperties(TestNode *node)
    {
        if (!node)
            return 1;

        if (node->color == TestNode::Color::kRed) {
            if (node->left && node->left->color == TestNode::Color::kRed) {
                R_ASSERT(false && "Red violation: Left child is Red");
                return -1;
            }
            if (node->right && node->right->color == TestNode::Color::kRed) {
                R_ASSERT(false && "Red violation: Right child is Red");
                return -1;
            }
        }

        if (node->left && node->left->key > node->key)
            return -1;
        if (node->right && node->right->key < node->key)
            return -1;

        int lh = ValidateRBProperties(node->left);
        int rh = ValidateRBProperties(node->right);

        if (lh == -1 || rh == -1 || lh != rh) {
            R_ASSERT(false && "Black height violation");
            return -1;
        }

        return lh + (node->color == TestNode::Color::kBlack ? 1 : 0);
    }
};

TEST_F(IntrusiveRBTreeTest, LargeSequentialInsert)
{
    constexpr int kCount = 5000;

    for (int i = 0; i < kCount; ++i) {
        tree.Insert(NewNode(i, i));
    }

    EXPECT_EQ(0, tree.Min()->key);
    EXPECT_EQ(kCount - 1, tree.Max()->key);

    EXPECT_EQ(2500, tree.Find(2500)->payload);
    EXPECT_EQ(4999, tree.Find(4999)->payload);
    ValidateRBProperties(tree.Root());
}

TEST_F(IntrusiveRBTreeTest, LargeReverseInsert)
{
    constexpr int kCount = 5000;
    for (int i = kCount - 1; i >= 0; --i) {
        tree.Insert(NewNode(i, i));
    }

    EXPECT_EQ(0, tree.Min()->key);
    EXPECT_EQ(kCount - 1, tree.Max()->key);
    ValidateRBProperties(tree.Root());
}

TEST_F(IntrusiveRBTreeTest, RandomizedTorture)
{
    SimpleRandom rng(12345);
    constexpr int kOperations = 10000;
    constexpr int kKeySpace   = 2000;

    bool exists[kKeySpace]     = {false};
    TestNode *nodes[kKeySpace] = {nullptr};

    for (int i = 0; i < kOperations; ++i) {
        int key = rng.next() % kKeySpace;
        int op  = rng.next() % 100;

        if (op < 60) {
            if (!exists[key]) {
                TestNode *n = NewNode(key, i);
                tree.Insert(n);
                nodes[key]  = n;
                exists[key] = true;
            }
        } else if (op < 90) {
            if (exists[key]) {
                tree.Delete(nodes[key]);
                nodes[key]  = nullptr;
                exists[key] = false;
            }
        } else {
            TestNode *found = tree.Find(key);
            if (exists[key]) {
                EXPECT_NEQ(nullptr, found);
                if (found) {
                    EXPECT_EQ(key, found->key);
                }
            } else {
                EXPECT_EQ(nullptr, found);
            }
        }

        ValidateRBProperties(tree.Root());
    }

    for (int i = 0; i < kKeySpace; ++i) {
        TestNode *found = tree.Find(i);
        if (exists[i]) {
            EXPECT_NEQ(nullptr, found);
        } else {
            EXPECT_EQ(nullptr, found);
        }
    }

    ValidateRBProperties(tree.Root());
}

TEST_F(IntrusiveRBTreeTest, HeavyBucketUsage)
{
    constexpr int kKeys        = 10;
    constexpr int kItemsPerKey = 100;

    for (int k = 0; k < kKeys; ++k) {
        TestNode *head = NewNode(k, 0);
        tree.Insert(head);

        TestNode *current_tail = head;
        for (int i = 1; i < kItemsPerKey; ++i) {
            TestNode *item = NewNode(k, i);

            current_tail->next = item;
            item->next         = nullptr;

            current_tail = item;
        }
    }

    for (int k = 0; k < kKeys; ++k) {
        TestNode *head = tree.Find(k);
        ASSERT(head != nullptr);
        EXPECT_EQ(k, head->key);
        EXPECT_EQ(0, head->payload);

        int count      = 1;
        TestNode *curr = head->next;
        while (curr) {
            EXPECT_EQ(k, curr->key);
            EXPECT_EQ(count, curr->payload);
            count++;
            curr = curr->next;
        }
        EXPECT_EQ(kItemsPerKey, count);
    }

    TestNode *head5 = tree.Find(5);
    tree.Delete(5);
    EXPECT_EQ(nullptr, tree.Find(5));

    int count5 = 0;
    while (head5) {
        count5++;
        head5 = head5->next;
    }
    EXPECT_EQ(kItemsPerKey, count5);

    ValidateRBProperties(tree.Root());
}

TEST_F(IntrusiveRBTreeTest, DeleteMinMaxRepeatedly)
{
    constexpr int kCount = 1000;
    for (int i = 0; i < kCount; ++i) tree.Insert(NewNode(i));
    ValidateRBProperties(tree.Root());

    int deleted_count = 0;
    while (tree.Min()) {
        TestNode *min = tree.Min();
        EXPECT_EQ(deleted_count, min->key);
        tree.Delete(min);
        deleted_count++;
    }
    EXPECT_EQ(kCount, deleted_count);
    EXPECT_TRUE(tree.Min() == nullptr);

    for (int i = 0; i < kCount; ++i) tree.Insert(NewNode(i));
    ValidateRBProperties(tree.Root());

    deleted_count = 0;
    while (tree.Max()) {
        TestNode *max = tree.Max();
        EXPECT_EQ(kCount - 1 - deleted_count, max->key);
        tree.Delete(max);
        deleted_count++;
    }
    EXPECT_EQ(kCount, deleted_count);
}

TEST_F(IntrusiveRBTreeTest, NodeMoveScenario)
{
    TestNode *n = NewNode(10, 123);
    tree.Insert(n);
    EXPECT_NEQ(nullptr, tree.Find(10));

    tree.Delete(n);
    EXPECT_EQ(nullptr, tree.Find(10));

    n->key = 20;

    tree.Insert(n);
    EXPECT_EQ(nullptr, tree.Find(10));
    EXPECT_NEQ(nullptr, tree.Find(20));
    EXPECT_EQ(123, tree.Find(20)->payload);

    ValidateRBProperties(tree.Root());
}

}  // namespace data_structures::test
