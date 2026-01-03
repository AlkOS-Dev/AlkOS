#ifndef KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_

#include <alkos/structs.h>
#include <defines.hpp>

namespace Syscall
{
FAST_CALL int SysThreadCreate(Thread *, thread_func_t, void *) { return 0; }

FAST_CALL int SysThreadJoin(Thread *) { return 0; }

FAST_CALL int SysThreadDetach(Thread *) { return 0; }

FAST_CALL void SysThreadExit(void *) {}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
