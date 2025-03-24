#ifndef ALKOS_LIBC_INCLUDE_SYS_TIME_H_
#define ALKOS_LIBC_INCLUDE_SYS_TIME_H_

#include <stdint.h>
#include <extensions/types.hpp>

typedef struct Timezone {
    i16 west_offset_minutes;
    i16 dst_time_offset_minutes;
    i16 dst_time_start_seconds;
    i16 dst_time_end_seconds;
} timezone;

typedef struct TimeVal {
    u64 seconds;
    u64 nanoseconds;
} time_val;

#endif  // ALKOS_LIBC_INCLUDE_SYS_TIME_H_
