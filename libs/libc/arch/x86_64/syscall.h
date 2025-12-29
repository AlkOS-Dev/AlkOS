#ifndef LIBS_LIBC_ARCH_X86_64_SYSCALL_H_
#define LIBS_LIBC_ARCH_X86_64_SYSCALL_H_

#include "defines.h"
#include "macro.hpp"
#include "types.hpp"

/**
 * @file syscall.h
 * @brief Userspace syscall interface for x86_64
 *
 * This header provides inline assembly wrappers for making system calls.
 * On x86_64, we use the int 0x80 instruction with the following convention:
 * - RAX: syscall number
 * - RDI: arg0
 * - RSI: arg1
 * - RDX: arg2
 * - R10: arg3
 * - R8:  arg4
 * - R9:  arg5
 * - Return value: RAX
 */

#define syscall0(num)                                                                             \
    ({                                                                                            \
        size_t __ret;                                                                             \
        __asm__ volatile("int    $0x80\n\t" : "=a"(__ret) : "0"((size_t)(num)) : "memory", "cc"); \
        __ret;                                                                                    \
    })

#define syscall1(num, arg0)                                        \
    ({                                                             \
        size_t __ret;                                              \
        __asm__ volatile("int    $0x80\n\t"                        \
                         : "=a"(__ret)                             \
                         : "0"((size_t)(num)), "D"((size_t)(arg0)) \
                         : "memory", "cc");                        \
        __ret;                                                     \
    })

#define syscall2(num, arg0, arg1)                                                       \
    ({                                                                                  \
        size_t __ret;                                                                   \
        __asm__ volatile("int    $0x80\n\t"                                             \
                         : "=a"(__ret)                                                  \
                         : "0"((size_t)(num)), "D"((size_t)(arg0)), "S"((size_t)(arg1)) \
                         : "memory", "cc");                                             \
        __ret;                                                                          \
    })

#define syscall3(num, arg0, arg1, arg2)                                                  \
    ({                                                                                   \
        size_t __ret;                                                                    \
        __asm__ volatile("int    $0x80\n\t"                                              \
                         : "=a"(__ret)                                                   \
                         : "0"((size_t)(num)), "D"((size_t)(arg0)), "S"((size_t)(arg1)), \
                           "d"((size_t)(arg2))                                           \
                         : "memory", "cc");                                              \
        __ret;                                                                           \
    })

#define syscall4(num, arg0, arg1, arg2, arg3)                                            \
    ({                                                                                   \
        size_t __ret;                                                                    \
        register size_t __r10 asm("r10") = (size_t)(arg3);                               \
        __asm__ volatile("int    $0x80\n\t"                                              \
                         : "=a"(__ret)                                                   \
                         : "0"((size_t)(num)), "D"((size_t)(arg0)), "S"((size_t)(arg1)), \
                           "d"((size_t)(arg2)), "r"(__r10)                               \
                         : "memory", "cc");                                              \
        __ret;                                                                           \
    })

#define syscall5(num, arg0, arg1, arg2, arg3, arg4)                                      \
    ({                                                                                   \
        size_t __ret;                                                                    \
        register size_t __r10 asm("r10") = (size_t)(arg3);                               \
        register size_t __r8 asm("r8")   = (size_t)(arg4);                               \
        __asm__ volatile("int    $0x80\n\t"                                              \
                         : "=a"(__ret)                                                   \
                         : "0"((size_t)(num)), "D"((size_t)(arg0)), "S"((size_t)(arg1)), \
                           "d"((size_t)(arg2)), "r"(__r10), "r"(__r8)                    \
                         : "memory", "cc");                                              \
        __ret;                                                                           \
    })

#define syscall6(num, arg0, arg1, arg2, arg3, arg4, arg5)                                \
    ({                                                                                   \
        size_t __ret;                                                                    \
        register size_t __r10 asm("r10") = (size_t)(arg3);                               \
        register size_t __r8 asm("r8")   = (size_t)(arg4);                               \
        register size_t __r9 asm("r9")   = (size_t)(arg5);                               \
        __asm__ volatile("int    $0x80\n\t"                                              \
                         : "=a"(__ret)                                                   \
                         : "0"((size_t)(num)), "D"((size_t)(arg0)), "S"((size_t)(arg1)), \
                           "d"((size_t)(arg2)), "r"(__r10), "r"(__r8), "r"(__r9)         \
                         : "memory", "cc");                                              \
        __ret;                                                                           \
    })

#define _SYSCALL_PP_NARG(...)                             _SYSCALL_PP_NARG_(__VA_ARGS__ __VA_OPT__(, ) _SYSCALL_PP_RSEQ_N())
#define _SYSCALL_PP_NARG_(...)                            _SYSCALL_PP_ARG_N(__VA_ARGS__)
#define _SYSCALL_PP_ARG_N(_1, _2, _3, _4, _5, _6, N, ...) N
#define _SYSCALL_PP_RSEQ_N()                              6, 5, 4, 3, 2, 1, 0

#define _SYSCALL_DISPATCH_(n, ...) CONCAT(syscall, n)(__VA_ARGS__)

#define _SYSCALL_DISPATCH(num, ...) \
    _SYSCALL_DISPATCH_(_SYSCALL_PP_NARG(__VA_ARGS__), num __VA_OPT__(, ) __VA_ARGS__)

#endif  // LIBS_LIBC_ARCH_X86_64_SYSCALL_H_
