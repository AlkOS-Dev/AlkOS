#ifndef KERNEL_SRC_SCHEDULING_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICY_HPP_

#include <types.h>
#include <defines.hpp>

#include "thread.hpp"

namespace Sched
{
enum class SchedulingPolicy {
    kUberTask_PQ_P0,
    kDrivers_PQ_P1,
    kUrgentTasks_PQ_P2,
    kNormalTasks_RR_P3,
    kBackgroundTasks_RR_P4,
    kLast,
};

struct Policy {
    struct {
        Thread *(*pick_thread)(void *);
    } cbs;
    void *self;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICY_HPP_
