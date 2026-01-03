#ifndef KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_

#include "array.hpp"
#include "scheduling/policy.hpp"
#include "scheduling/thread.hpp"

#include <data_structures/priority_queues/bitmap_pq.hpp>

namespace Sched
{

class PriorityQueuePolicy : public PolicyImpl
{
    public:
    static constexpr u8 kMaxPriority = 64;

    // ------------------------------
    // Class creation
    // ------------------------------

    PriorityQueuePolicy()  = default;
    ~PriorityQueuePolicy() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F Thread *PickNextTask() { return priority_queue_.DeleteMin(); }

    FORCE_INLINE_F void AddTask(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);
        ASSERT_EQ(thread->state, ThreadState::kReady);
        ASSERT_LT(thread->flags.priority, kMaxPriority);

        priority_queue_.Insert(thread, thread->flags.priority);
    }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::BitmapPriorityQueue<Thread, kMaxPriority> priority_queue_{};
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_
