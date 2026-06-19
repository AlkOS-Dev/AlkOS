// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

/**
 * @file crt0.cpp
 * @brief C Runtime Startup (CRT0) for userspace programs
 */

#include "internal/crt0.hpp"
#include "internal/stdio.hpp"

BEGIN_DECL_C

int main();

void (*atexit_funcs[MAX_ATEXIT_FUNCS])(void);
size_t atexit_func_count = 0;

/**
 * @brief Program entry point
 */
USED NO_RET SECTION(.text.start) void _start()
{
    InitStdio();             // Initialize standard I/O streams
    InitializeRuntime();     // Initialize runtime and global constructors
    const int res = main();  // Call the user program's main function
    FinalizeRuntime();       // Finalize runtime and global destructors

    __platform_proc_exit(res);
    __builtin_unreachable();
}

END_DECL_C
