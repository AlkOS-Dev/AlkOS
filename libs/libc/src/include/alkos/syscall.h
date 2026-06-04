#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYSCALL_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYSCALL_H_

#include <syscall.h>
#include "internal/macro.hpp"

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

    /* Panic/Program termination */
    kSysPanic,

    /* File descriptor syscalls */
    kSysOpen,
    kSysClose,
    kSysRead,
    kSysWrite,
    kSysSeek,
    kSysDup,
    kSysDupTo,
    kSysReadDirectory,
    kSysFileInfo,

    /* Threads, processes */
    kThreadCreate,
    kThreadExit,
    kThreadJoin,
    kThreadDetach,
    kProcExit,
    kProcAbort,
    kNanoSleep,
    kNanoSleepUntil,
    kExec,
    kKill,
    kWait,

    /* Video Syscalls */
    kSysCreateGraphicSession,
    kSysBlit,

    /* Power Management Syscalls */
    kSysPower,

    kSysMax,
};

#define SYSCALL_NAME(name, num, ret_type, ...) \
    ret_type __platform_##name(FOR_EACH_PAIR(MERGE, __VA_ARGS__))

#define SYSCALL_VOID_NAME(name, num, ...) void __platform_##name(FOR_EACH_PAIR(MERGE, __VA_ARGS__))

#define DEFINE_SYSCALL(name, num, ret_type, ...)                                             \
    SYSCALL_NAME(name, num, ret_type __VA_OPT__(, ) __VA_ARGS__)                             \
    {                                                                                        \
        return (ret_type)_SYSCALL_DISPATCH(num, FOR_EACH_PAIR(GET_SECOND_ARG, __VA_ARGS__)); \
    }

#define DEFINE_SYSCALL_VOID(name, num, ...)                                       \
    SYSCALL_VOID_NAME(name, num __VA_OPT__(, ) __VA_ARGS__)                       \
    {                                                                             \
        (void)_SYSCALL_DISPATCH(num, FOR_EACH_PAIR(GET_SECOND_ARG, __VA_ARGS__)); \
    }

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYSCALL_H_
