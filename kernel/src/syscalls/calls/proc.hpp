#ifndef KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_

#include <defines.hpp>
#include "hardware/core_local.hpp"
#include "modules/scheduling.hpp"
#include "modules/video.hpp"
#include "scheduling/local_lock.hpp"

namespace Syscall
{
FAST_CALL void SysExit(int status) {}

FAST_CALL void SysAbort() {}

FAST_CALL int SysExec(const char *path, u64 *pid)
{
    if (path == nullptr) {
        return -1;
    }

    const auto result = SchedulingModule::Get().GetTaskMgr().Exec(path);

    if (!result) {
        return -1;
    }

    if (pid != nullptr) {
        *pid = *reinterpret_cast<const u64 *>(&result.value());
    }

    return 0;
}

FAST_CALL void SysFocusTransfer(Sched::Pid target_child)
{
    auto &wm = VideoModule::Get().GetWindowManager();
    wm.SetFocus(target_child);
}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
