#ifndef KERNEL_SRC_SCHEDULING_WAIT_QUEUE_HPP_
#define KERNEL_SRC_SCHEDULING_WAIT_QUEUE_HPP_

#include <data_structures/intrusive_linked_list.hpp>

#include <concepts.hpp>

#include "alkos/sys/proc.h"

namespace Sched
{

template <class T, int kIntrusiveLevel>
class WaitQueue
{
    using HookT = data_structures::IntrusiveDoubleListNode<T, kIntrusiveLevel>;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    WaitQueue()
    {
        wait_list_.PushFront(&sentinel_front);
        wait_list_.PushBack(&sentinel_back);
    }
    ~WaitQueue() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FAST_CALL void Remove(T *item)
    {
        item->HookT::prev->HookT::next = item->HookT::next;
        item->HookT::next->HookT::prev = item->HookT::prev;

        item->HookT::next = nullptr;
        item->HookT::prev = nullptr;
    }

    NODISCARD FORCE_INLINE_F bool Contains(T *item) { return wait_list_.Contains(item); }

    NODISCARD FORCE_INLINE_F bool IsEmpty()
    {
        return wait_list_.Front()->HookT::next == &sentinel_back;
    }

    FORCE_INLINE_F void EnqueueFirst(T *item)
    {
        wait_list_.PopFront();
        wait_list_.PushFront(item);
        wait_list_.PushFront(&sentinel_front);
    }

    FORCE_INLINE_F void EnqueueLast(T *item)
    {
        wait_list_.PopBack();
        wait_list_.PushBack(item);
        wait_list_.PushBack(&sentinel_back);
    }

    FORCE_INLINE_F NODISCARD T *Dequeue()
    {
        wait_list_.PopFront();
        T *result = wait_list_.PopFront();
        wait_list_.PushFront(&sentinel_front);
        return result;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T sentinel_front;
    T sentinel_back;
    data_structures::IntrusiveDoubleList<T, kIntrusiveLevel> wait_list_;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_WAIT_QUEUE_HPP_
