#ifndef KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_

#include <alkos/structs.h>
#include <defines.hpp>

#include "modules/scheduling.hpp"
#include "modules/timing.hpp"

namespace Syscall
{
FAST_CALL int SysThreadCreate(Thread *, thread_func_t, void *) { return 0; }

FAST_CALL int SysThreadJoin(Thread *) { return 0; }

FAST_CALL int SysThreadDetach(Thread *) { return 0; }

FAST_CALL void SysThreadExit(void *) {}

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
