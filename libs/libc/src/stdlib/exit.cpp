// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "stdlib.h"

#include "alkos/sys/proc.h"
#include "crt0.hpp"

extern void (*atexit_funcs[])(void);
extern size_t atexit_func_count;

void exit(int status)
{
    FinalizeRuntime();
    __platform_proc_exit(status);
    __builtin_unreachable();
}

int atexit(void (*func)(void))
{
    atexit_func_count = __sync_fetch_and_add(&atexit_func_count, 1);
    if (atexit_func_count >= MAX_ATEXIT_FUNCS) {
        return -1;
    }

    atexit_funcs[atexit_func_count] = func;
    return 0;
}
