#ifndef LIBC_INCLUDE_SYS_TIME_H_
#define LIBC_INCLUDE_SYS_TIME_H_

#include <stdint.h>
#include <extensions/types.hpp>

typedef struct Timezone {
    i16 west_offset_minutes;
    i16 dst_time_offset_minutes;
    i16 dst_time_start_seconds;
    i16 dst_time_end_seconds;
} timezone;

typedef struct TimerVal {
    u64 seconds;
    u64 nanoseconds;
} timer_val;

#endif  // LIBC_INCLUDE_SYS_TIME_H_
