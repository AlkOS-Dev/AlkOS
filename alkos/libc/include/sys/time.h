#ifndef LIBC_INCLUDE_SYS_TIME_H_
#define LIBC_INCLUDE_SYS_TIME_H_

#include <stdint.h>

typedef struct Timezone {
    int32_t west_offset_seconds;
    int32_t dst_time_offset_seconds;
    int32_t dst_time_start_seconds;
    int32_t dst_time_end_seconds;
} timezone;

typedef struct TimerVal {
    uint64_t seconds;
    uint64_t nanoseconds;
} timer_val;

#endif  // LIBC_INCLUDE_SYS_TIME_H_
