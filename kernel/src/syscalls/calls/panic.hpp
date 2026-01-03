#ifndef KERNEL_SRC_SYSCALLS_CALLS_PANIC_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_PANIC_HPP_

#include <defines.h>

#include "hal/panic.hpp"

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
    hal::KernelPanic(msg);
    __builtin_unreachable();
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_PANIC_HPP_
