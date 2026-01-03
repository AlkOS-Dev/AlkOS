#ifndef KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_
#define KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_

#include <array.hpp>
#include <defines.hpp>

#include "policy.hpp"
#include "thread.hpp"

#include "policies/priority_queue_policy.hpp"
#include "policies/round_robin_policy.hpp"

namespace Sched
{

class Scheduler
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Scheduler();
    ~Scheduler() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void AddReadyThread(Thread *thread);

    Thread *Schedule();

    void Yield();

    void ConvertToScheduling();

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    // ------------------------------
    // Class fields
    // ------------------------------

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
