#ifndef KERNEL_SRC_SYSCALLS_CALLS_PANIC_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_PANIC_HPP_

#include <defines.h>

#include "hal/panic.hpp"
#include "hardware/core_local.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "trace_framework.hpp"

namespace Syscall
{

// ------------------------------
// Panic Syscall
// ------------------------------

/**
 * @brief Trigger a kernel panic with a message
 * @param msg Panic message
 */
NO_RET FORCE_INLINE_F void SysPanic(const char *msg)
{
    auto pid = hardware::GetRunningPid();

    if (msg == nullptr || *msg == '\0') {
        TRACE_WARN_GENERAL("User Panic in PID %llu: No message provided", pid.id);
    } else {
        TRACE_WARN_GENERAL("User Panic in PID %llu: %s", pid.id, msg);
    }

    SchedulingModule::Get().GetTaskMgr().CommitSuicide();

    __builtin_unreachable();
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_PANIC_HPP_
