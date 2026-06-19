// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INTERNAL_CRT0_HPP_
#define LIBS_LIBC_SRC_INTERNAL_CRT0_HPP_

#include "defines.h"
#include "types.h"

#define MAX_ATEXIT_FUNCS 32

BEGIN_DECL_C

extern void (*__preinit_array_start[])(void) WEAK;
extern void (*__preinit_array_end[])(void) WEAK;
extern void (*__init_array_start[])(void) WEAK;
extern void (*__init_array_end[])(void) WEAK;
extern void (*__fini_array_start[])(void) WEAK;
extern void (*__fini_array_end[])(void) WEAK;

void _init(void);
void _fini(void);

extern void (*atexit_funcs[])(void);
extern size_t atexit_func_count;

FORCE_INLINE_F void InitializeRuntime()
{
    // Call pre-initialization functions
    for (size_t i = 0; i < static_cast<size_t>(__preinit_array_end - __preinit_array_start); ++i) {
        __preinit_array_start[i]();
    }

    // Call the main initialization function
    _init();

    // Call initialization functions
    for (size_t i = 0; i < static_cast<size_t>(__init_array_end - __init_array_start); ++i) {
        __init_array_start[i]();
    }
}

FORCE_INLINE_F void FinalizeRuntime()
{
    // Call registered atexit functions in reverse order
    for (size_t i = atexit_func_count; i > 0; --i) {
        atexit_funcs[i - 1]();
    }

    // Call finalization functions in reverse order
    for (size_t i = static_cast<size_t>(__fini_array_end - __fini_array_start); i > 0; --i) {
        __fini_array_start[i - 1]();
    }

    // Call the main finalization function
    _fini();
}

END_DECL_C

#endif  // LIBS_LIBC_SRC_INTERNAL_CRT0_HPP_
