#ifndef KERNEL_SRC_SCHEDULING_WAIT_QUEUE_HPP_
#define KERNEL_SRC_SCHEDULING_WAIT_QUEUE_HPP_

#include <concepts.hpp>
#include <data_structures/intrusive_linked_list.hpp>
#include <template/special_members.hpp>
#include "alkos/sys/proc.h"

namespace Sched
{

template <class T, int kIntrusiveLevel>
class WaitQueue : template_lib::NoCopy
{
    using NodeT = data_structures::IntrusiveDoubleListNode<T, kIntrusiveLevel>;

    public:
    WaitQueue()
    {
        root_.NodeT::next = &root_;
        root_.NodeT::prev = &root_;
    }

    ~WaitQueue() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FAST_CALL void Remove(T *item)
    {
        item->NodeT::prev->NodeT::next = item->NodeT::next;
        item->NodeT::next->NodeT::prev = item->NodeT::prev;

        item->NodeT::next = nullptr;
        item->NodeT::prev = nullptr;
    }

    NODISCARD FORCE_INLINE_F bool Contains(T *item)
    {
        T *current = root_.NodeT::next;
        T *end     = &root_;

        while (current != end) {
            if (current == item) {
                return true;
            }
            current = current->NodeT::next;
        }
        return false;
    }

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return root_.NodeT::next == &root_; }

    FORCE_INLINE_F void EnqueueFirst(T *item) { InsertBetween_(&root_, root_.NodeT::next, item); }

    FORCE_INLINE_F void EnqueueLast(T *item) { InsertBetween_(root_.NodeT::prev, &root_, item); }

    NODISCARD FORCE_INLINE_F T *Dequeue()
    {
        if (IsEmpty()) {
            return nullptr;
        }

        T *item = root_.NodeT::next;
        Remove(item);
        return item;
    }

    private:
    FORCE_INLINE_F void InsertBetween_(T *prev_node, T *next_node, T *item)
    {
        item->NodeT::prev = prev_node;
        item->NodeT::next = next_node;

        prev_node->NodeT::next = item;
        next_node->NodeT::prev = item;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    T root_;
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_WAIT_QUEUE_HPP_
