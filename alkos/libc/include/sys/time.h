#ifndef LIBC_INCLUDE_SYS_TIME_H_
#define LIBC_INCLUDE_SYS_TIME_H_

#include <stdint.h>
#include <extensions/types.hpp>

// ------------------------------
// Known time types
// ------------------------------

enum ClockType {
    kTimeUtc = 1,
    kProcTime,
    kLastClockType,
};

// ------------------------------
// Time structs
// ------------------------------

typedef struct Timezone {
    i16 west_offset_minutes;
    i16 dst_time_offset_minutes;
    u16 dst_time_start_seconds;
    u16 dst_time_end_seconds;
} timezone;

typedef struct TimeVal {
    u64 seconds;
    u64 remainder;
} time_val;

#endif  // LIBC_INCLUDE_SYS_TIME_H_
