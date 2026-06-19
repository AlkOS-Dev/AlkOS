// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <data_structures/linked_list.hpp>
#include <mem/cyclic_allocator.hpp>
#include <test_module/test.hpp>

// ------------------------------
// SingleLinkedList
// ------------------------------

using IntSingleList = data_structures::SingleLinkedList<int>;

class SingleLinkedListTest : public TestGroupBase
{
    public:
    IntSingleList list;
};

TEST_F(SingleLinkedListTest, EmptyList)
{
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(0_size, list.Size());
    EXPECT_EQ(nullptr, list.GetHead());
}

TEST_F(SingleLinkedListTest, PushFront)
{
    list.PushFront(1);
    EXPECT_EQ(1_size, list.Size());
    EXPECT_EQ(1, list.Front());

    list.PushFront(2);
    EXPECT_EQ(2_size, list.Size());
    EXPECT_EQ(2, list.Front());

    list.PushFront(0);
    EXPECT_EQ(3_size, list.Size());
    EXPECT_EQ(0, list.Front());
}

TEST_F(SingleLinkedListTest, PopFront)
{
    list.PushFront(3);
    list.PushFront(2);
    list.PushFront(1);

    list.PopFront();
    EXPECT_EQ(2_size, list.Size());
    EXPECT_EQ(2, list.Front());

    list.PopFront();
    EXPECT_EQ(1_size, list.Size());
    EXPECT_EQ(3, list.Front());

    list.PopFront();
    EXPECT_TRUE(list.Empty());
}

TEST_F(SingleLinkedListTest, Clear)
{
    list.PushFront(1);
    list.PushFront(2);
    list.PushFront(3);

    EXPECT_EQ(3_size, list.Size());
    list.Clear();
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(0_size, list.Size());
}

TEST_F(SingleLinkedListTest, Iterator)
{
    list.PushFront(3);
    list.PushFront(2);
    list.PushFront(1);

    int expected = 1;
    for (auto value : list) {
        EXPECT_EQ(expected, value);
        ++expected;
    }
}

TEST_F(SingleLinkedListTest, MoveConstructor)
{
    list.PushFront(2);
    list.PushFront(1);

    IntSingleList otherList(std::move(list));
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(2_size, otherList.Size());
    EXPECT_EQ(1, otherList.Front());
}

TEST_F(SingleLinkedListTest, MoveAssignment)
{
    list.PushFront(2);
    list.PushFront(1);

    IntSingleList otherList;
    otherList.PushFront(99);

    otherList = std::move(list);
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(2_size, otherList.Size());
    EXPECT_EQ(1, otherList.Front());
}

TEST_F(SingleLinkedListTest, CustomAllocator)
{
    data_structures::StaticSingleLinkedList<int, 8> list;

    list.PushFront(3);
    list.PushFront(2);
    list.PushFront(1);

    EXPECT_EQ(3_size, list.Size());
    EXPECT_EQ(1, list.Front());

    list.Clear();
    EXPECT_EQ(0_size, list.Size());
}

TEST_F(SingleLinkedListTest, InsertAfter)
{
    auto *node1 = list.PushFront(1);
    auto *node2 = list.InsertAfter(node1, 2);
    list.InsertAfter(node2, 3);

    EXPECT_EQ(3_size, list.Size());

    int expected = 1;
    for (auto value : list) {
        EXPECT_EQ(expected, value);
        ++expected;
    }
}

TEST_F(SingleLinkedListTest, StructureSizes)
{
    EXPECT_EQ(16_size, sizeof(IntSingleList::Node));
    EXPECT_EQ(16_size, sizeof(IntSingleList));
}

// ------------------------------
// DoubleLinkedList
// ------------------------------

using IntDoubleList = data_structures::DoubleLinkedList<int>;

class DoubleLinkedListTest : public TestGroupBase
{
    public:
    IntDoubleList list;
};

TEST_F(DoubleLinkedListTest, EmptyList)
{
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(0_size, list.Size());
    EXPECT_EQ(nullptr, list.GetHead());
    EXPECT_EQ(nullptr, list.GetTail());
}

TEST_F(DoubleLinkedListTest, PushFrontAndBack)
{
    list.PushFront(1);
    EXPECT_EQ(1_size, list.Size());
    EXPECT_EQ(1, list.Front());
    EXPECT_EQ(1, list.Back());

    list.PushBack(2);
    EXPECT_EQ(2_size, list.Size());
    EXPECT_EQ(1, list.Front());
    EXPECT_EQ(2, list.Back());

    list.PushFront(0);
    EXPECT_EQ(3_size, list.Size());
    EXPECT_EQ(0, list.Front());
    EXPECT_EQ(2, list.Back());
}

TEST_F(DoubleLinkedListTest, PopFrontAndBack)
{
    list.PushBack(1);
    list.PushBack(2);
    list.PushBack(3);

    list.PopFront();
    EXPECT_EQ(2_size, list.Size());
    EXPECT_EQ(2, list.Front());

    list.PopBack();
    EXPECT_EQ(1_size, list.Size());
    EXPECT_EQ(2, list.Back());

    list.PopBack();
    EXPECT_TRUE(list.Empty());
}

TEST_F(DoubleLinkedListTest, RemoveNode)
{
    auto *node1 = list.PushBack(1);
    auto *node2 = list.PushBack(2);
    auto *node3 = list.PushBack(3);

    list.Remove(node2);
    EXPECT_EQ(2_size, list.Size());
    EXPECT_EQ(1, list.Front());
    EXPECT_EQ(3, list.Back());

    list.Remove(node1);
    EXPECT_EQ(1_size, list.Size());
    EXPECT_EQ(3, list.Front());

    list.Remove(node3);
    EXPECT_TRUE(list.Empty());
}

TEST_F(DoubleLinkedListTest, MoveToFront)
{
    auto *node1 = list.PushBack(1);
    auto *node2 = list.PushBack(2);
    auto *node3 = list.PushBack(3);

    list.MoveToFront(node3);
    EXPECT_EQ(3, list.Front());
    EXPECT_EQ(2, list.Back());

    list.MoveToFront(node1);
    EXPECT_EQ(1, list.Front());
}

TEST_F(DoubleLinkedListTest, MoveToBack)
{
    auto *node1 = list.PushBack(1);
    auto *node2 = list.PushBack(2);
    auto *node3 = list.PushBack(3);

    list.MoveToBack(node1);
    EXPECT_EQ(2, list.Front());
    EXPECT_EQ(1, list.Back());

    list.MoveToBack(node3);
    EXPECT_EQ(3, list.Back());
}

TEST_F(DoubleLinkedListTest, Clear)
{
    list.PushBack(1);
    list.PushBack(2);
    list.PushBack(3);

    EXPECT_EQ(3_size, list.Size());
    list.Clear();
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(0_size, list.Size());
}

TEST_F(DoubleLinkedListTest, Iterator)
{
    list.PushBack(1);
    list.PushBack(2);
    list.PushBack(3);

    int expected = 1;
    for (auto value : list) {
        EXPECT_EQ(expected, value);
        ++expected;
    }
}

TEST_F(DoubleLinkedListTest, MoveConstructor)
{
    list.PushBack(1);
    list.PushBack(2);

    IntDoubleList otherList(std::move(list));
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(2_size, otherList.Size());
    EXPECT_EQ(1, otherList.Front());
    EXPECT_EQ(2, otherList.Back());
}

TEST_F(DoubleLinkedListTest, MoveAssignment)
{
    list.PushBack(1);
    list.PushBack(2);

    IntDoubleList otherList;
    otherList.PushBack(99);

    otherList = std::move(list);
    EXPECT_TRUE(list.Empty());
    EXPECT_EQ(2_size, otherList.Size());
    EXPECT_EQ(1, otherList.Front());
    EXPECT_EQ(2, otherList.Back());
}

TEST_F(DoubleLinkedListTest, CustomAllocator)
{
    data_structures::StaticDoubleLinkedList<int, 8> list;

    list.PushBack(1);
    list.PushBack(2);
    list.PushBack(3);

    EXPECT_EQ(3_size, list.Size());
    EXPECT_EQ(1, list.Front());
    EXPECT_EQ(3, list.Back());

    list.Clear();
    EXPECT_EQ(0_size, list.Size());
}

TEST_F(DoubleLinkedListTest, InsertAfter)
{
    auto *node1 = list.PushBack(1);
    auto *node2 = list.InsertAfter(node1, 2);
    list.InsertAfter(node2, 3);

    EXPECT_EQ(3_size, list.Size());

    int expected = 1;
    for (auto value : list) {
        EXPECT_EQ(expected, value);
        ++expected;
    }
}

TEST_F(DoubleLinkedListTest, StructureSizes)
{
    EXPECT_EQ(24_size, sizeof(IntDoubleList::Node));  // 20 + 4 bytes padding
    EXPECT_EQ(24_size, sizeof(IntDoubleList));
}
