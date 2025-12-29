#ifndef LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_H_
#define LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_H_

#include "macro.hpp"
#include "syscall.h"

/**
 * @brief Syscall numbers
 *
 * This is the single source of truth for syscall numbers.
 */
enum SyscallNumber {
    /* Time related syscalls */
    kSysGetClockValue = 0,
    kSysGetClockTicksInSecond,
    kSysGetTimezone,

    /* Debug IO */
    kSysDebugWrite,
    kSysDebugReadLine,
    kSysWriteConsole,

    /* Panic/Program termination */
    kSysPanic,

    kSysMax,
};

#define DEFINE_SYSCALL(name, num, ret_type, ...)                                             \
    ret_type __platform_##name(FOR_EACH_PAIR(MERGE, __VA_ARGS__))                            \
    {                                                                                        \
        return (ret_type)_SYSCALL_DISPATCH(num, FOR_EACH_PAIR(GET_SECOND_ARG, __VA_ARGS__)); \
    }

#define DEFINE_SYSCALL_VOID(name, num, ...)                                       \
    void __platform_##name(FOR_EACH_PAIR(MERGE, __VA_ARGS__))                     \
    {                                                                             \
        (void)_SYSCALL_DISPATCH(num, FOR_EACH_PAIR(GET_SECOND_ARG, __VA_ARGS__)); \
    }

/**
 * @brief This file collects all the system calls that are used by the libc.
 *
 * Calls should be specified in other header files and then included here.
 */

#include <sys/calls/time.h>

#endif  // LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_H_
