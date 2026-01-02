#ifndef LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_THREAD_H_
#define LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_THREAD_H_

#include "defines.h"
#include "platform.h"
#include "sys/thread.h"

// ------------------------------
// System calls
// ------------------------------

BEGIN_DECL_C

FAST_CALL int ThreadCreate(Thread *thread, ThreadFlags flags, thread_func_t f, void *arg){return }

FAST_CALL int ThreadJoin(Thread *thread)
{
}

FAST_CALL int ThreadDetach(Thread *thread) {}

FAST_CALL void ThreadExit(void *retval) {}

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_THREAD_H_
