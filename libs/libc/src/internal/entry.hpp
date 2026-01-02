#ifndef LIBS_LIBC_SRC_INTERNAL_ENTRY_HPP_
#define LIBS_LIBC_SRC_INTERNAL_ENTRY_HPP_

#include "defines.h"
#include "internal/stdio.hpp"

BEGIN_DECL_C

extern int main();

/**
 * @brief Entry point to initialize C runtime
 *
 * This function initializes standard I/O streams and then calls the main function.
 */
void _start()
{
    // Initialize standard I/O streams
    InitStdio();

    // Call main function
    main();
}

END_DECL_C

#endif  // LIBS_LIBC_SRC_INTERNAL_ENTRY_HPP_
