/**
 * @file crt0.cpp
 * @brief C Runtime Startup (CRT0) for userspace programs
 */

#include "defines.h"
#include "internal/stdio.hpp"

BEGIN_DECL_C

extern void (*__preinit_array_start[])(void) WEAK;
extern void (*__preinit_array_end[])(void) WEAK;
extern void (*__init_array_start[])(void) WEAK;
extern void (*__init_array_end[])(void) WEAK;
extern void (*__fini_array_start[])(void) WEAK;
extern void (*__fini_array_end[])(void) WEAK;

void _init(void);
void _fini(void);
int main();

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
    // Call finalization functions in reverse order
    for (size_t i = static_cast<size_t>(__fini_array_end - __fini_array_start); i > 0; --i) {
        __fini_array_start[i - 1]();
    }

    // Call the main finalization function
    _fini();
}

/**
 * @brief Program entry point
 */
USED NO_RET SECTION(.text.start) void _start()
{
    InitStdio();             // Initialize standard I/O streams
    InitializeRuntime();     // Initialize runtime and global constructors
    const int res = main();  // Call the user program's main function
    FinalizeRuntime();       // Finalize runtime and global destructors

    // TODO : Handle program exit properly
    __platform_proc_exit(res);  // Not implemented
    while (true) {
        size_t i = 0;
        for (i = 0; i < 1'000'000;) {
            ++i;
        }
    }
    __builtin_unreachable();
}

END_DECL_C
