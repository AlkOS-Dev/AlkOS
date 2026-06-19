// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SCHEDULING_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICY_HPP_

#include <assert.h>
#include <types.h>
#include <concepts.hpp>
#include <defines.hpp>

namespace Sched
{
struct Thread;
struct ThreadFlags;

// ------------------------------
// SchedulingPolicy
// ------------------------------

enum class SchedulingPolicy : u8 {
    kUberTask_PQ_P0 = 0,
    kDrivers_PQ_P1,
    kUrgentTasks_PQ_P2,
    kNormalTasks_MQAPS_P3,
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
        u64 (*get_preempt_time)(void *, Thread *);
        bool (*is_first_higher_priority)(void *, Thread *, Thread *);
        bool (*validate_flags)(void *, const ThreadFlags *);
        void (*on_thread_yield)(void *, Thread *);
        void (*on_periodic_update)(void *, u64 current_time_ns);
        void (*remove_task)(void *, Thread *);
    } cbs;
    void *self;
};

// ------------------------------
// Helpers
// ------------------------------

struct PolicyImpl {
    NODISCARD Thread *PickNextTask() { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }
    void AddTask(Thread *) { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }
    NODISCARD u64 GetPreemptTime(Thread *) { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }
    NODISCARD bool IsFirstHigherPriority(Thread *, Thread *) { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }
    NODISCARD bool ValidateThreadFlags(const ThreadFlags *) { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }
    void RemoveTask(Thread *) { R_FAIL_ALWAYS("NOT_IMPLEMENTED"); }

    // Event callbacks
    void OnThreadYield(Thread *) {}
    void OnPeriodicUpdate(u64) {}
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
u64 GetPreemptTimeImpl(void *self, Thread *thread)
{
    const auto rr = static_cast<T *>(self);
    return rr->GetPreemptTime(thread);
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
bool IsFirstHigherPriorityImpl(void *self, Thread *first, Thread *second)
{
    const auto rr = static_cast<T *>(self);
    return rr->IsFirstHigherPriority(first, second);
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
bool ValidateThreadFlagsImpl(void *self, const ThreadFlags *flags)
{
    const auto rr = static_cast<T *>(self);
    return rr->ValidateThreadFlags(flags);
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
void OnThreadYieldImpl(void *self, Thread *thread)
{
    const auto policy = static_cast<T *>(self);
    policy->OnThreadYield(thread);
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
void OnPeriodicUpdateImpl(void *self, u64 current_time_ns)
{
    const auto policy = static_cast<T *>(self);
    policy->OnPeriodicUpdate(current_time_ns);
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
void RemoveTaskImpl(void *self, Thread *thread)
{
    const auto policy = static_cast<T *>(self);
    policy->RemoveTask(thread);
}

template <class T>
    requires std::derived_from<T, PolicyImpl>
NODISCARD FAST_CALL Policy PreparePolicy(T *self)
{
    Policy policy{};

    policy.self                         = self;
    policy.cbs.pick_next_task           = PickNextTaskImpl<T>;
    policy.cbs.add_task                 = AddTaskImpl<T>;
    policy.cbs.get_preempt_time         = GetPreemptTimeImpl<T>;
    policy.cbs.is_first_higher_priority = IsFirstHigherPriorityImpl<T>;
    policy.cbs.validate_flags           = ValidateThreadFlagsImpl<T>;
    policy.cbs.on_thread_yield          = OnThreadYieldImpl<T>;
    policy.cbs.on_periodic_update       = OnPeriodicUpdateImpl<T>;
    policy.cbs.remove_task              = RemoveTaskImpl<T>;

    return policy;
}

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICY_HPP_
