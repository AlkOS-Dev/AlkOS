#ifndef KERNEL_SRC_SYSCALLS_CALLS_INPUT_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_INPUT_HPP_

#include "alkos/sys/input.h"
#include "modules/hardware.hpp"

#include <modules/scheduling.hpp>
#include <modules/video.hpp>

namespace Syscall
{

/**
 * @brief Syscall Handler: Get Key State
 *
 * @param vk Virtual Key to check
 * @return true if the key is currently pressed, false otherwise
 */
FORCE_INLINE_F bool SysGetKeyState(VirtualKey vk)
{
    auto res = ::SchedulingModule::Get().GetProcesses().GetCurrentProcess();
    R_ASSERT_TRUE(static_cast<bool>(res), "SysGetKeyState: No current process found");
    if (::VideoModule::Get().GetWindowManager().GetActiveSessionFocusedPid() != (*res)->pid) {
        return false;
    }
    return ::HardwareModule::Get().GetPs2Keyboard().GetKeyState(vk);
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_INPUT_HPP_
