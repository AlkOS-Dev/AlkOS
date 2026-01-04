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

    NODISCARD FORCE_INLINE_F u64 GetPreemptTime(Thread *thread)
    {
        static constexpr u64 kPreemptTimeNs = 5'000'000;  // 5ms

        ASSERT_NOT_NULL(thread);

        if (thread->state != ThreadState::kRunning) {
            return kPreemptTimeNs;
        }

        const u64 cpu_time_ns = thread->CalculateCpuTime();
        return cpu_time_ns < kPreemptTimeNs ? kPreemptTimeNs - cpu_time_ns : 0;
    }

    NODISCARD FORCE_INLINE_F bool IsFirstHigherPriority(Thread *first, Thread *second)
    {
        ASSERT_NOT_NULL(first);
        ASSERT_NOT_NULL(second);

        return first->flags.priority > second->flags.priority;
    }

    NODISCARD bool ValidateThreadFlags(const ThreadFlags *flags)
    {
        return flags->priority >= kMaxPriority;
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
