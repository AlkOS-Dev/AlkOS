#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_INTRUSIVE_LINKED_LIST_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_INTRUSIVE_LINKED_LIST_HPP_

#include <concepts.hpp>
#include <defines.hpp>
#include <template/special_members.hpp>

namespace data_structures
{
template <class T>
struct IntrusiveListNode {
    T *next;
};

template <class T>
struct IntrusiveDoubleListNode {
    T *next;
    T *prev;
};

template <class T>
    requires std::derived_from<T, IntrusiveListNode<T>>
class FrontIntrusiveList : template_lib::NoCopy
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    FrontIntrusiveList() = default;
    explicit FrontIntrusiveList(T *item) : head_(item) {}
    ~FrontIntrusiveList() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return head_ == nullptr; }

    FORCE_INLINE_F void PushFront(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            head_      = item;
            item->next = nullptr;
            return;
        }

        item->next = head_;
        head_      = item;
    }

    NODISCARD FORCE_INLINE_F T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item    = head_;
        head_      = item->next;
        item->next = nullptr;

        return item;
    }

    NODISCARD FORCE_INLINE_F T *Front() { return head_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    T *head_{};
};

template <class T>
    requires std::derived_from<T, IntrusiveListNode<T>>
class IntrusiveList : template_lib::NoCopy
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    IntrusiveList()  = default;
    ~IntrusiveList() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return head_ == nullptr; }

    FORCE_INLINE_F void PushBack(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        tail_->next = item;
        item->next  = nullptr;
        tail_       = item;
    }

    FORCE_INLINE_F void PushFront(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        item->next = head_;
        head_      = item;
    }

    NODISCARD FORCE_INLINE_F T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item    = head_;
        head_      = item->next;
        item->next = nullptr;

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
        head_      = item;
        tail_      = item;
        item->next = nullptr;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T *head_{};
    T *tail_{};
};

template <class T>
    requires std::derived_from<T, IntrusiveDoubleListNode<T>>
class IntrusiveDoubleList : template_lib::NoCopy
{
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

    FORCE_INLINE_F void PushBack(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        tail_->next = item;
        item->next  = nullptr;
        item->prev  = tail_;
        tail_       = item;
    }

    FORCE_INLINE_F void PushFront(T *item)
    {
        ASSERT_NOT_NULL(item);

        if (IsEmpty()) {
            InitList_(item);
            return;
        }

        item->next  = head_;
        item->prev  = nullptr;
        head_->prev = item;
        head_       = item;
    }

    NODISCARD FORCE_INLINE_F T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item = head_;
        head_   = item->next;

        if (head_) {
            head_->prev = nullptr;
        } else {
            tail_ = nullptr;
        }

        item->next = nullptr;
        item->prev = nullptr;
        return item;
    }

    NODISCARD FORCE_INLINE_F T *PopBack()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item = tail_;
        tail_   = item->prev;

        if (tail_) {
            tail_->next = nullptr;
        } else {
            head_ = nullptr;
        }

        item->prev = nullptr;
        item->next = nullptr;
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
        head_      = item;
        tail_      = item;
        item->next = nullptr;
        item->prev = nullptr;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T *head_{};
    T *tail_{};
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_INTRUSIVE_LINKED_LIST_HPP_
