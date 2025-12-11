#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_LINKED_LIST_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_LINKED_LIST_HPP_

#include <stddef.h>
#include <allocators/custom_allocator_wrapper.hpp>
#include <new.hpp>
#include <template/utils.hpp>
#include <type_traits.hpp>
#include <utility.hpp>

namespace data_structures
{

namespace internal
{
template <typename T, bool kIsDoubleLinked = true>
struct LinkedListNode {
    T data;
    OPTIONAL_FIELD(kIsDoubleLinked, LinkedListNode *) prev;
    LinkedListNode *next;

    template <typename... Args>
    explicit LinkedListNode(Args &&...args)
        : data(std::forward<Args>(args)...), prev{}, next(nullptr)
    {
    }
};
}  // namespace internal

/**
 * @brief A linked list that can be either singly or doubly-linked
 *
 * @tparam T The type of elements stored in the list.
 * @tparam AllocatorT The allocator type
 * @tparam kIsDoubleLinked If true, creates a doubly-linked list; if false, singly-linked
 */
template <typename T, typename AllocatorT, bool kIsDoubleLinked = true>
class LinkedList
{
    public:
    using Node = internal::LinkedListNode<T, kIsDoubleLinked>;

    class Iterator
    {
        public:
        using value_type = T;
        using reference  = T &;
        using pointer    = T *;

        explicit Iterator(Node *node) : node_(node) {}

        reference operator*() const { return node_->data; }
        pointer operator->() const { return &node_->data; }

        Iterator &operator++()
        {
            node_ = node_->next;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator &operator--()
            requires(kIsDoubleLinked)
        {
            node_ = node_->prev;
            return *this;
        }

        Iterator operator--(int)
            requires(kIsDoubleLinked)
        {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const Iterator &other) const { return node_ == other.node_; }
        bool operator!=(const Iterator &other) const { return node_ != other.node_; }

        Node *GetNode() const { return node_; }

        private:
        Node *node_;
    };

    class ConstIterator
    {
        public:
        using value_type = const T;
        using reference  = const T &;
        using pointer    = const T *;

        explicit ConstIterator(const Node *node) : node_(node) {}

        reference operator*() const { return node_->data; }
        pointer operator->() const { return &node_->data; }

        ConstIterator &operator++()
        {
            node_ = node_->next;
            return *this;
        }

        ConstIterator operator++(int)
        {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        ConstIterator &operator--()
            requires(kIsDoubleLinked)
        {
            node_ = node_->prev;
            return *this;
        }

        ConstIterator operator--(int)
            requires(kIsDoubleLinked)
        {
            ConstIterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const ConstIterator &other) const { return node_ == other.node_; }
        bool operator!=(const ConstIterator &other) const { return node_ != other.node_; }

        const Node *GetNode() const { return node_; }

        private:
        const Node *node_;
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    LinkedList() : head_(nullptr), tail_{}, size_(0) {}
    ~LinkedList() { Clear(); }

    LinkedList(const LinkedList &)            = delete;
    LinkedList &operator=(const LinkedList &) = delete;

    LinkedList(LinkedList &&other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_), allocator_(other.allocator_)
    {
        other.head_ = nullptr;
        if constexpr (kIsDoubleLinked) {
            other.tail_ = nullptr;
        }
        other.size_ = 0;
    }

    LinkedList &operator=(LinkedList &&other) noexcept
    {
        if (this != &other) {
            Clear();
            head_       = other.head_;
            tail_       = other.tail_;
            size_       = other.size_;
            allocator_  = other.allocator_;
            other.head_ = nullptr;
            if constexpr (kIsDoubleLinked) {
                other.tail_ = nullptr;
            }
            other.size_ = 0;
        }
        return *this;
    }

    // Capacity
    size_t Size() const { return size_; }
    bool Empty() const { return size_ == 0; }

    // Element access
    T &Front() { return head_->data; }
    const T &Front() const { return head_->data; }

    T &Back()
        requires(kIsDoubleLinked)
    {
        return tail_->data;
    }

    const T &Back() const
        requires(kIsDoubleLinked)
    {
        return tail_->data;
    }

    // Iterators
    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
    ConstIterator begin() const { return ConstIterator(head_); }
    ConstIterator end() const { return ConstIterator(nullptr); }

    ConstIterator cbegin() const { return ConstIterator(head_); }
    ConstIterator cend() const { return ConstIterator(nullptr); }

    Node *PushFront(const T &value) { return EmplaceFront(value); }
    Node *PushFront(T &&value) { return EmplaceFront(std::move(value)); }

    template <typename... Args>
    Node *EmplaceFront(Args &&...args)
    {
        Node *node = AllocateNode(std::forward<Args>(args)...);
        if (!node) {
            return nullptr;
        }

        if constexpr (kIsDoubleLinked) {
            node->prev = nullptr;
        }
        node->next = head_;

        if constexpr (kIsDoubleLinked) {
            if (head_ != nullptr) {
                head_->prev = node;
            }
        }

        head_ = node;

        if constexpr (kIsDoubleLinked) {
            if (tail_ == nullptr) {
                tail_ = node;
            }
        }

        ++size_;
        return node;
    }

    template <typename... Args>
    Node *PushBack(Args &&...args)
        requires(kIsDoubleLinked)
    {
        Node *node = AllocateNode(std::forward<Args>(args)...);
        if (!node) {
            return nullptr;
        }

        node->prev = tail_;
        node->next = nullptr;

        if (tail_ != nullptr) {
            tail_->next = node;
        }

        tail_ = node;

        if (head_ == nullptr) {
            head_ = node;
        }

        ++size_;
        return node;
    }

    Node *InsertAfter(Node *after, const T &value) { return EmplaceAfter(after, value); }
    Node *InsertAfter(Node *after, T &&value) { return EmplaceAfter(after, std::move(value)); }

    template <typename... Args>
    Node *EmplaceAfter(Node *after, Args &&...args)
    {
        if (!after) {
            return nullptr;
        }

        Node *node = AllocateNode(std::forward<Args>(args)...);
        if (!node) {
            return nullptr;
        }

        node->next = after->next;
        if constexpr (kIsDoubleLinked) {
            node->prev = after;

            if (after->next != nullptr) {
                after->next->prev = node;
            } else {
                tail_ = node;
            }
        }
        after->next = node;

        ++size_;
        return node;
    }

    void PopFront()
    {
        if (head_ == nullptr) {
            return;
        }

        Node *node = head_;
        head_      = head_->next;

        if constexpr (kIsDoubleLinked) {
            if (head_ != nullptr) {
                head_->prev = nullptr;
            } else {
                tail_ = nullptr;
            }
        }

        DeallocateNode(node);
        --size_;
    }

    void PopBack()
        requires(kIsDoubleLinked)
    {
        if (tail_ == nullptr) {
            return;
        }

        Node *node = tail_;
        tail_      = tail_->prev;

        if (tail_ != nullptr) {
            tail_->next = nullptr;
        } else {
            head_ = nullptr;
        }

        DeallocateNode(node);
        --size_;
    }

    void Remove(Node *node)
        requires(kIsDoubleLinked)
    {
        if (!node) {
            return;
        }

        if (node->prev != nullptr) {
            node->prev->next = node->next;
        } else {
            head_ = node->next;
        }

        if (node->next != nullptr) {
            node->next->prev = node->prev;
        } else {
            tail_ = node->prev;
        }

        node->prev = nullptr;
        node->next = nullptr;

        DeallocateNode(node);
        --size_;
    }

    void MoveToFront(Node *node)
        requires(kIsDoubleLinked)
    {
        if (!node || node == head_) {
            return;
        }

        // Unlink from current position
        if (node->prev != nullptr) {
            node->prev->next = node->next;
        }

        if (node->next != nullptr) {
            node->next->prev = node->prev;
        } else {
            tail_ = node->prev;
        }

        // Insert at front
        node->prev = nullptr;
        node->next = head_;

        if (head_ != nullptr) {
            head_->prev = node;
        }

        head_ = node;

        if (tail_ == nullptr) {
            tail_ = node;
        }
    }

    void MoveToBack(Node *node)
        requires(kIsDoubleLinked)
    {
        if (!node || node == tail_) {
            return;
        }

        // Unlink from current position
        if (node->prev != nullptr) {
            node->prev->next = node->next;
        } else {
            head_ = node->next;
        }

        if (node->next != nullptr) {
            node->next->prev = node->prev;
        }

        // Insert at back
        node->prev = tail_;
        node->next = nullptr;

        if (tail_ != nullptr) {
            tail_->next = node;
        }

        tail_ = node;

        if (head_ == nullptr) {
            head_ = node;
        }
    }

    void Clear()
    {
        Node *node = head_;
        while (node != nullptr) {
            Node *next = node->next;
            DeallocateNode(node);
            node = next;
        }

        head_ = nullptr;
        if constexpr (kIsDoubleLinked) {
            tail_ = nullptr;
        }
        size_ = 0;
    }

    Node *GetHead() const { return head_; }

    Node *GetTail() const
        requires(kIsDoubleLinked)
    {
        return tail_;
    }

    // ------------------------------
    // Helpers
    // ------------------------------

    private:
    template <typename... Args>
    Node *AllocateNode(Args &&...args)
    {
        return allocator_.template Allocate<Node>(std::forward<Args>(args)...);
    }

    void DeallocateNode(Node *node)
    {
        if (node) {
            node->~Node();
            allocator_.template Deallocate<Node>(node);
        }
    }

    // ------------------------------
    // Field members
    // ------------------------------

    private:
    Node *head_;
    OPTIONAL_FIELD(kIsDoubleLinked, Node *) tail_;
    size_t size_;
    NO_UNIQUE_ADDRESS AllocatorT allocator_{};
};

template <
    typename T, typename AllocatorT = allocators::KMallocAllocator<internal::LinkedListNode<T>>>
using DoubleLinkedList = LinkedList<T, AllocatorT>;

template <typename T, size_t kSize>
using StaticDoubleLinkedList =
    DoubleLinkedList<T, allocators::CyclicAllocatorWrapper<internal::LinkedListNode<T>, kSize>>;

template <
    typename T,
    typename AllocatorT = allocators::KMallocAllocator<internal::LinkedListNode<T, false>>>
using SingleLinkedList = LinkedList<T, AllocatorT, false>;

template <typename T, size_t kSize>
using StaticSingleLinkedList = SingleLinkedList<
    T, allocators::CyclicAllocatorWrapper<internal::LinkedListNode<T, false>, kSize>>;

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_LINKED_LIST_HPP_
