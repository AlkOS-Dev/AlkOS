#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_THREAD_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_THREAD_H_

#include <defines.h>
#include <stdbool.h>
#include <types.h>

typedef void (*thread_func_t)(void *);

enum SchedulingPolicy {
    kUberTask_PQ_P0 = 0,
    kDrivers_PQ_P1,
    kUrgentTasks_PQ_P2,
    kNormalTasks_RR_P3,
    kBackgroundTasks_RR_P4,
    kLast,
};

typedef struct PACK {
    enum SchedulingPolicy policy : 8;
    u8 priority : 8;
    bool preserve_floats : 1;
    bool detached : 1;
    u64 padding : 46;
} ThreadFlags;

typedef struct {
    u64 tid;
    ThreadFlags flags;
} Thread;

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_THREAD_H_
