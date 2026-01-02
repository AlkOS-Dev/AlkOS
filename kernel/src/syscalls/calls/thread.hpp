#ifndef KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_

#include <alkos/structs.h>
#include <defines.hpp>

namespace Syscall
{
FAST_CALL int ThreadCreate(Thread *thread, thread_func_t f, void *arg) { return 0; }

FAST_CALL int ThreadJoin(Thread *thread) { return 0; }

FAST_CALL int ThreadDetach(Thread *thread) { return 0; }

FAST_CALL void ThreadExit(void *retval) {}
}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_THREAD_HPP_
