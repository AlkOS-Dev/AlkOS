/**
 * @file crt0.cpp
 * @brief C Runtime Startup (CRT0) for userspace programs
 */

#include "defines.h"
#include "internal/stdio.hpp"

BEGIN_DECL_C

extern int main();

/**
 * @brief Program entry point
 */
USED NO_RET SECTION(.text.start) void _start()
{
    InitStdio();             // Initialize standard I/O streams
    const int res = main();  // Call the user program's main function

    // TODO : Handle program exit properly
    __platform_proc_exit(res);  // Not implemented
    while (true) {
        size_t i = 0;
        for (i = 0; i < 1'000'000;) {
            ++i;
        }
    }
}

END_DECL_C
