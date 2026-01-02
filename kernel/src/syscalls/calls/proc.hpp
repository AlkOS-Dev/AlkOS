#ifndef KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_

#include <defines.hpp>

namespace Syscall
{
FAST_CALL void SysExit(int status) {}
FAST_CALL void SysAbort() {}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_PROC_HPP_
