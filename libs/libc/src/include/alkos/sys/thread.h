#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_THREAD_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_THREAD_H_

#include "alkos/thread.h"
#include "defines.h"
#include "platform.h"

// ------------------------------
// System calls
// ------------------------------

BEGIN_DECL_C

FAST_CALL int ThreadCreate(Thread *thread, ThreadFlags flags, thread_func_t f, void *arg)
{
    thread->flags = flags;
    return __platform_thread_create(thread, f, arg);
}

FAST_CALL int ThreadJoin(Thread *thread) { return __platform_thread_join(thread); }

FAST_CALL int ThreadDetach(Thread *thread) { return __platform_thread_detach(thread); }

FAST_CALL void ThreadExit(void *retval) { __platform_thread_exit(retval); }

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_THREAD_H_
