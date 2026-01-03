#ifndef KERNEL_SRC_SCHEDULING_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICY_HPP_

#include <assert.h>
#include <types.h>
#include <concepts.hpp>
#include <defines.hpp>

namespace Sched
{
class Thread;

// ------------------------------
// SchedulingPolicy
// ------------------------------

enum class SchedulingPolicy : u8 {
    kUberTask_PQ_P0 = 0,
    kDrivers_PQ_P1,
    kUrgentTasks_PQ_P2,
    kNormalTasks_RR_P3,
    kBackgroundTasks_RR_P4,
    kLast,
};

// ------------------------------
// Policy
// ------------------------------

struct Policy {
    struct {
        Thread *(*pick_next_task)(void *);
        void (*add_task)(void *, Thread *);
    } cbs;
    void *self;
};

// ------------------------------
// Helpers
// ------------------------------

struct PolicyImpl {
    NODISCARD Thread *PickNextTask() { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }
    void AddTask(Thread *thread) { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }
};

template <class T>
    requires std::derived_from<T, PolicyImpl>
Thread *PickNextTaskImpl(void *self)
{
    const auto rr = static_cast<T *>(self);
    return rr->PickNextTask();
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
void AddTaskImpl(void *self, Thread *thread)
{
    const auto rr = static_cast<T *>(self);
    rr->AddTask(thread);
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
NODISCARD FAST_CALL Policy PreparePolicy(T *self)
{
    Policy policy{};

    policy.self               = self;
    policy.cbs.pick_next_task = PickNextTaskImpl<T>;
    policy.cbs.add_task       = AddTaskImpl<T>;

    return policy;
}

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICY_HPP_
