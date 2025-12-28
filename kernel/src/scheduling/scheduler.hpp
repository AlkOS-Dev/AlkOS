#ifndef KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_
#define KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_

#include <defines.hpp>
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

    void Schedule();

    void ConvertToScheduling();

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    NODISCARD FORCE_INLINE_F Thread *GetNext_();

    // ------------------------------
    // Class fields
    // ------------------------------

    Thread *threads_{};
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_SCHEDULER_HPP_
