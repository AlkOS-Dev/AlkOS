// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_INTRUSIVE_LINKED_LIST_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_INTRUSIVE_LINKED_LIST_HPP_

#include <concepts.hpp>
#include <defines.hpp>
#include <template/special_members.hpp>

namespace data_structures
{
template <class T, int kIntrusiveLevel>
struct IntrusiveListNode {
    T *next;
};

template <class T, int kIntrusiveLevel>
struct IntrusiveDoubleListNode {
    T *next;
    T *prev;
};

template <int kIntrusiveLevel, class T>
    requires std::derived_from<T, IntrusiveListNode<T, kIntrusiveLevel>>
class FrontIntrusiveListView : template_lib::NoCopy
{
    using NodeType = IntrusiveListNode<T, kIntrusiveLevel>;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    FrontIntrusiveListView() = delete;
    explicit FrontIntrusiveListView(T *&item) : head_(item) {}
    ~FrontIntrusiveListView() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return head_ == nullptr; }

    FORCE_INLINE_F void PushFront(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            head_             = item;
            item->NodeT::next = nullptr;
            return;
        }

        item->NodeT::next = head_;
        head_             = item;
    }

    NODISCARD FORCE_INLINE_F T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item           = head_;
        head_             = item->NodeT::next;
        item->NodeT::next = nullptr;

        return item;
    }

    NODISCARD FORCE_INLINE_F T *Front() { return head_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    T *&head_{};
};

template <class T, int kIntrusiveLevel>
    requires std::derived_from<T, IntrusiveDoubleListNode<T, kIntrusiveLevel>>
class FronIntrusiveDoubleListView : template_lib::NoCopy
{
    using NodeT = IntrusiveDoubleListNode<T, kIntrusiveLevel>;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    FronIntrusiveDoubleListView() = delete;
    explicit FronIntrusiveDoubleListView(T *&item) : head_(item) {}
    ~FronIntrusiveDoubleListView() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool Contains(T *item)
    {
        T *node = head_;

        while (node) {
            if (node == item) {
                return true;
            }
            node = node->NodeT::next;
        }

        return false;
    }

    FORCE_INLINE_F void Remove(T *item)
    {
        ASSERT_NOT_NULL(item);
        ASSERT_FALSE(IsEmpty());
        ASSERT_TRUE(Contains(item));

        if (head_ == item) {
            head_ = item->NodeT::next;
            if (head_) {
                head_->NodeT::prev = nullptr;
            }

            item->NodeT::next = nullptr;
            item->NodeT::prev = nullptr;
            return;
        }

        item->NodeT::prev->NodeT::next = item->NodeT::next;

        if (item->NodeT::next) {
            item->NodeT::next->NodeT::prev = item->NodeT::prev;
        }

        item->NodeT::prev = nullptr;
        item->NodeT::next = nullptr;
    }

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return head_ == nullptr; }

    FORCE_INLINE_F void PushFront(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        item->NodeT::next  = head_;
        item->NodeT::prev  = nullptr;
        head_->NodeT::prev = item;
        head_              = item;
    }

    NODISCARD FORCE_INLINE_F T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item = head_;
        head_   = item->NodeT::next;

        if (head_) {
            head_->NodeT::prev = nullptr;
        }

        item->NodeT::next = nullptr;
        item->NodeT::prev = nullptr;
        return item;
    }

    NODISCARD FORCE_INLINE_F T *Front() { return head_; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    FORCE_INLINE_F void InitList_(T *item)
    {
        head_             = item;
        item->NodeT::next = nullptr;
        item->NodeT::prev = nullptr;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T *&head_;
};

template <class T, int kIntrusiveLevel>
    requires std::derived_from<T, IntrusiveListNode<T, kIntrusiveLevel>>
class IntrusiveList : template_lib::NoCopy
{
    public:
    using NodeT = IntrusiveListNode<T, kIntrusiveLevel>;
    // ------------------------------
    // Class creation
    // ------------------------------

    IntrusiveList()  = default;
    ~IntrusiveList() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool Contains(T *item)
    {
        T *node = head_;

        while (node) {
            if (node == item) {
                return true;
            }
            node = node->NodeT::next;
        }

        return false;
    }

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return head_ == nullptr; }

    FORCE_INLINE_F void PushBack(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        tail_->NodeT::next = item;
        item->NodeT::next  = nullptr;
        tail_              = item;
    }

    FORCE_INLINE_F void PushFront(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        item->NodeT::next = head_;
        head_             = item;
    }

    NODISCARD FORCE_INLINE_F T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item           = head_;
        head_             = item->NodeT::next;
        item->NodeT::next = nullptr;

        // Not Needed due to IsEmpty() checks
        // if (head_ == nullptr) {
        //     tail_ = nullptr;
        // }

        return item;
    }

    NODISCARD FORCE_INLINE_F T *Front() { return head_; }

    NODISCARD FORCE_INLINE_F T *Back()
    {
        if (IsEmpty()) {
            return nullptr;
        }
        return tail_;
    }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    FORCE_INLINE_F void InitList_(T *item)
    {
        head_             = item;
        tail_             = item;
        item->NodeT::next = nullptr;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T *head_{};
    T *tail_{};
};

template <class T, int kIntrusiveLevel>
    requires std::derived_from<T, IntrusiveDoubleListNode<T, kIntrusiveLevel>>
class IntrusiveDoubleList : template_lib::NoCopy
{
    using NodeT = IntrusiveDoubleListNode<T, kIntrusiveLevel>;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    IntrusiveDoubleList()  = default;
    ~IntrusiveDoubleList() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return head_ == nullptr; }

    NODISCARD FORCE_INLINE_F bool Contains(T *item)
    {
        T *node = head_;

        while (node) {
            if (node == item) {
                return true;
            }
            node = node->NodeT::next;
        }

        return false;
    }

    FORCE_INLINE_F void Remove(T *item)
    {
        ASSERT_NOT_NULL(item);
        ASSERT_FALSE(IsEmpty());
        ASSERT_TRUE(Contains(item));

        if (head_ == tail_) {
            ASSERT_EQ(head_, item);
            head_ = tail_ = nullptr;

            item->NodeT::next = nullptr;
            item->NodeT::prev = nullptr;
            return;
        }

        if (head_ == item) {
            head_              = item->NodeT::next;
            head_->NodeT::prev = nullptr;

            item->NodeT::next = nullptr;
            item->NodeT::prev = nullptr;
            return;
        }

        if (tail_ == item) {
            tail_              = item->NodeT::prev;
            tail_->NodeT::next = nullptr;

            item->NodeT::prev = nullptr;
            item->NodeT::next = nullptr;
            return;
        }

        item->NodeT::prev->NodeT::next = item->NodeT::next;
        item->NodeT::next->NodeT::prev = item->NodeT::prev;

        item->NodeT::prev = nullptr;
        item->NodeT::next = nullptr;
    }

    FORCE_INLINE_F void PushBack(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        tail_->NodeT::next = item;
        item->NodeT::next  = nullptr;
        item->NodeT::prev  = tail_;
        tail_              = item;
    }

    FORCE_INLINE_F void PushFront(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        item->NodeT::next  = head_;
        item->NodeT::prev  = nullptr;
        head_->NodeT::prev = item;
        head_              = item;
    }

    NODISCARD FORCE_INLINE_F T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item = head_;
        head_   = item->NodeT::next;

        if (head_) {
            head_->NodeT::prev = nullptr;
        } else {
            tail_ = nullptr;
        }

        item->NodeT::next = nullptr;
        item->NodeT::prev = nullptr;
        return item;
    }

    NODISCARD FORCE_INLINE_F T *PopBack()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item = tail_;
        tail_   = item->NodeT::prev;

        if (tail_) {
            tail_->NodeT::next = nullptr;
        } else {
            head_ = nullptr;
        }

        item->NodeT::prev = nullptr;
        item->NodeT::next = nullptr;
        return item;
    }

    NODISCARD FORCE_INLINE_F T *Front() { return head_; }

    NODISCARD FORCE_INLINE_F T *Back() { return tail_; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    FORCE_INLINE_F void InitList_(T *item)
    {
        head_             = item;
        tail_             = item;
        item->NodeT::next = nullptr;
        item->NodeT::prev = nullptr;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T *head_{};
    T *tail_{};
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_INTRUSIVE_LINKED_LIST_HPP_
