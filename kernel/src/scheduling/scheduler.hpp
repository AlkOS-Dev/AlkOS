#ifndef KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_
#define KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_

#include <array.hpp>
#include <defines.hpp>

#include "policy.hpp"
#include "thread.hpp"

namespace Sched
{

class Scheduler
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Scheduler()  = default;
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

    std::array<Policy, static_cast<size_t>(SchedulingPolicy::kLast)> policies_{};
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_
