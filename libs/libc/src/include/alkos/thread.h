#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_THREAD_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_THREAD_H_

#include <defines.h>
#include <stdbool.h>
#include <types.h>

typedef void (*thread_func_t)(void *);

typedef struct PACK {
    bool PreserveFloats : 1;
    u32 padding : 31;
} ThreadFlags;

typedef struct {
    u64 tid;
    ThreadFlags flags;
} Thread;

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_THREAD_H_
