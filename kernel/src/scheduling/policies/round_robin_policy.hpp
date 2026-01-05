#ifndef KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_

#include <assert.h>
#include <data_structures/intrusive_linked_list.hpp>
#include <defines.hpp>

#include "scheduling/policy.hpp"
#include "scheduling/thread.hpp"

namespace Sched
{

class RoundRobinPolicy : public PolicyImpl
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    RoundRobinPolicy()  = default;
    ~RoundRobinPolicy() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F Thread *PickNextTask() { return threads_.PopFront(); }

    FORCE_INLINE_F void AddTask(Thread *thread)
    {
        ASSERT_NOT_NULL(thread);
        ASSERT_EQ(thread->state, ThreadState::kReady);
        threads_.PushBack(thread);
    }

    NODISCARD FORCE_INLINE_F u64 GetPreemptTime(Thread *thread)
    {
        static constexpr u64 kPreemptTimeNs = 20'000'000;  // 20ms

        ASSERT_NOT_NULL(thread);

        if (thread->state != ThreadState::kRunning) {
            return kPreemptTimeNs;
        }

        const u64 cpu_time_ns = thread->CalculateCpuTime();
        return cpu_time_ns < kPreemptTimeNs ? kPreemptTimeNs - cpu_time_ns : 0;
    }

    NODISCARD FORCE_INLINE_F bool IsFirstHigherPriority(Thread *, Thread *) { return false; }

    NODISCARD bool ValidateThreadFlags(const ThreadFlags *) { return false; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::IntrusiveList<Thread> threads_{};
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_
