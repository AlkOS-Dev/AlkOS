#ifndef KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_

#include <alkos/structs.h>
#include <defines.hpp>

#include "modules/scheduling.hpp"
#include "modules/timing.hpp"

namespace Syscall
{
FAST_CALL int SysThreadCreate(Thread *thread, thread_func_t func, void *arg)
{
    if (thread == nullptr || func == nullptr) {
        return -1;
    }

    Sched::ThreadFlags flags{};
    flags.policy          = static_cast<Sched::SchedulingPolicy>(thread->flags.policy);
    flags.priority        = thread->flags.priority;
    flags.preserve_floats = thread->flags.preserve_floats;

    if (flags.policy < Sched::SchedulingPolicy::kNormalTasks_RR_P3) {
        return -1;  // User may only spawn normal and background tasks
    }

    if (SchedulingModule::Get().GetScheduler().ValidateThreadFlags(flags)) {
        return -1;  // Invalid thrad flags for given policy
    }

    const auto result = SchedulingModule::Get().GetTaskMgr().CreateUserThread(flags, func, arg);
    if (!result) {
        return -1;
    }

    const Sched::Tid tid = result.value();
    thread->tid          = *reinterpret_cast<const u64 *>(&tid);
    return 0;
}

FAST_CALL int SysThreadJoin(Thread *thread, void **retval)
{
    if (thread == nullptr) {
        return -1;
    }

    const auto result = SchedulingModule::Get().GetTaskMgr().JoinThread(
        *reinterpret_cast<Sched::Tid *>(&thread->tid)
    );

    if (!result) {
        *retval = nullptr;
        return -1;
    }

    *retval = result.value();
    return 0;
}

FAST_CALL int SysThreadDetach(Thread *thread)
{
    if (thread == nullptr) {
        return -1;
    }

    const auto result = SchedulingModule::Get().GetTaskMgr().DetachThread(
        *reinterpret_cast<Sched::Tid *>(&thread->tid)
    );
    return !result ? -1 : result;
}

FAST_CALL void SysThreadExit(void *retval)
{
    SchedulingModule::Get().GetTaskMgr().ThreadExit(retval);
}

FAST_CALL void SysNanoSleepUntil(const u64 systime_ns)
{
    SchedulingModule::Get().GetScheduler().NanoSleepUntil(systime_ns);
}

FAST_CALL void SysNanoSleep(const u64 time_ns)
{
    static constexpr u64 kSyscallCorrection = 200;
    SysNanoSleepUntil(
        TimingModule::Get().GetSystemTime().ReadLifeTimeNs() + time_ns - kSyscallCorrection
    );
}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
