#ifndef KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_

#include <defines.hpp>
#include "hardware/core_local.hpp"
#include "modules/scheduling.hpp"
#include "modules/video.hpp"
#include "scheduling/local_lock.hpp"

namespace Syscall
{
FAST_CALL void SysExit(const int status)
{
    SchedulingModule::Get().GetTaskMgr().ExitProcess(status);
}

FAST_CALL void SysAbort() { SchedulingModule::Get().GetTaskMgr().CommitSuicide(); }

FAST_CALL int SysKill(const u64 pid)
{
    const auto result = SchedulingModule::Get().GetTaskMgr().CommitMurder(
        *reinterpret_cast<const Sched::Pid *>(&pid)
    );
    return result ? 0 : -1;
}

FAST_CALL int SysWait(const u64 pid)
{
    const auto result = SchedulingModule::Get().GetTaskMgr().JoinProcess(
        *reinterpret_cast<const Sched::Pid *>(&pid)
    );
    return result ? result.value() : std::numeric_limits<int>::max();
}

FAST_CALL u64 SysExec(const char *path)
{
    if (path == nullptr) {
        return 0;
    }

    const auto result = SchedulingModule::Get().GetTaskMgr().Exec(path);

    if (!result) {
        return 0;
    }

    const auto pid = result.value();
    return *reinterpret_cast<const u64 *>(&pid);
}

FAST_CALL void SysFocusTransfer(Sched::Pid target_child)
{
    auto &wm = VideoModule::Get().GetWindowManager();
    wm.SetFocus(target_child);
}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
