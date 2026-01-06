#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_PROC_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_PROC_H_

#include "defines.h"
#include "platform.h"

// ------------------------------
// System calls
// ------------------------------

BEGIN_DECL_C

FAST_CALL void Exit(int status) { __platform_proc_exit(status); }

FAST_CALL void Abort() { __platform_proc_abort(); }

FAST_CALL int Exec(const char *path, u64 *pid) { return __platform_exec(path, pid); }

FAST_CALL int kill(u64 pid) { return __platform_kill(pid); }

FAST_CALL int wait(u64 pid) { return __platform_wait(pid); }

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_PROC_H_
