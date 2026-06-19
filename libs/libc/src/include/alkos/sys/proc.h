// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

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

FAST_CALL u64 Exec(const char *path) { return __platform_exec(path, false); }

FAST_CALL u64 ExecAsync(const char *path) { return __platform_exec(path, true); }

FAST_CALL int Kill(u64 pid) { return __platform_kill(pid); }

FAST_CALL int Wait(const u64 pid) { return __platform_wait(pid); }

FAST_CALL void *GetHeapStart() { return __platform_get_heap_start(); }

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_PROC_H_
