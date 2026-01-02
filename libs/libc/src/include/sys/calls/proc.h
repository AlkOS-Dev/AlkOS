#ifndef LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_PROC_H_
#define LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_PROC_H_

#include "defines.h"
#include "platform.h"

// ------------------------------
// System calls
// ------------------------------

BEGIN_DECL_C

FAST_CALL void Exit(int status) {}

FAST_CALL void Abort() {}

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_PROC_H_
