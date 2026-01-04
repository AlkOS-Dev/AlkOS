#ifndef KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_
#define KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_

#include <array.hpp>
#include <defines.hpp>
#include <hardware/core_mask.hpp>

#include "policy.hpp"
#include "thread.hpp"

#include "data_structures/maps/intrusive_rb_tree.hpp"
#include "hal/scheduling.hpp"
#include "hal/spinlock.hpp"
#include "hardware/core_local.hpp"
#include "policies/priority_queue_policy.hpp"
#include "policies/round_robin_policy.hpp"

namespace Sched
{

class Scheduler
{
    static constexpr u64 kMinDelta = 2'000;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Scheduler();
    ~Scheduler() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void InstallInterruptHandler();

    void AddReadyThread(Thread *thread);

    Thread *Schedule();

    Thread *ScheduleAndUpdateThreads(bool preempt, ThreadState thread_state);

    void Yield();

    void ConvertToScheduling();

    /* Should only be called inside syscall code or kernel thread code */
    void NanoSleepUntil(u64 systime_ns);

    // True if should preempt
    NODISCARD bool WakeUpTasks();

    NODISCARD Thread *TimerRoutine();

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    void PrepareNextTimerInterruptBeforeSwitchUnguarded_(Thread *next_thread);

    NODISCARD FORCE_INLINE_F const Policy &GetPolicy_(Thread *thread) const
    {
        ASSERT_NOT_NULL(thread);
        ASSERT_LT(
            static_cast<size_t>(thread->flags.policy), static_cast<size_t>(SchedulingPolicy::kLast)
        );

        return policies_[static_cast<size_t>(thread->flags.policy)];
    }

    NODISCARD FORCE_INLINE_F u64 GetPreemptTime_(Thread *thread) const
    {
        const auto &policy = GetPolicy_(thread);
        return policy.cbs.get_preempt_time(policy.self, thread);
    }

    void SetupNextTimeEvent_(u64 time_ns);

    NODISCARD FORCE_INLINE_F bool ShouldPreempt_(Thread *thread) const
    {
        return ShouldPreempt_(GetPreemptTime_(thread));
    }

    NODISCARD FORCE_INLINE_F bool IsFirstHigherPriority_(Thread *first, Thread *second) const
    {
        if (first->flags.policy == second->flags.policy) {
            const auto policy = GetPolicy_(first);
            return policy.cbs.is_first_higher_priority(policy.self, first, second);
        }

        return first->flags.policy > second->flags.policy;
    }

    NODISCARD FAST_CALL bool ShouldPreempt_(const u64 preempt_time_ns)
    {
        return preempt_time_ns == 0 || preempt_time_ns < kMinDelta;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    // Sleeping queue
    data_structures::IntrusiveRBTree<Thread, u64> sleep_queue_{};

    // Locking
    hal::Spinlock spinlock_{};

    // Policies
    PriorityQueuePolicy policy0_{};  // kUberTask_PQ_P0
    PriorityQueuePolicy policy1_{};  // kDrivers_PQ_P1
    PriorityQueuePolicy policy2_{};  // kUrgentTasks_PQ_P2
    RoundRobinPolicy policy3_{};     // kNormalTasks_RR_P3
    RoundRobinPolicy policy4_{};     // kBackgroundTasks_RR_P4

    // Abstraction
    std::array<Policy, static_cast<size_t>(SchedulingPolicy::kLast)> policies_{};
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_
