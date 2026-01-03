#ifndef KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_

#include <defines.hpp>
#include "hardware/core_local.hpp"
#include "modules/video.hpp"

namespace Syscall
{
FAST_CALL void SysExit(int status) {}
FAST_CALL void SysAbort() {}

FORCE_INLINE_F void SysFocusTransfer(Sched::Pid target_child)
{
    auto &wm = VideoModule::Get().GetWindowManager();
    wm.SetFocus(target_child);
}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
