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

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_PROC_H_
