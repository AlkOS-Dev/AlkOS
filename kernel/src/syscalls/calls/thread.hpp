#ifndef KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_

#include <alkos/structs.h>
#include <defines.hpp>

namespace Syscall
{
FAST_CALL int SysThreadCreate(Thread *thread, thread_func_t f, void *arg) { return 0; }

FAST_CALL int SysThreadJoin(Thread *thread) { return 0; }

FAST_CALL int SysThreadDetach(Thread *thread) { return 0; }

FAST_CALL void SysThreadExit(void *retval) {}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
