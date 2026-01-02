#ifndef LIBS_LIBC_SRC_INCLUDE_SYS_TIME_H_
#define LIBS_LIBC_SRC_INCLUDE_SYS_TIME_H_

#include <stdint.h>
#include <types.h>

// ------------------------------
// Known time types
// ------------------------------

typedef enum {
    kTimeUtc = 1,
    kProcTime,
    kProcTimePrecise,
    kLastClockType,
} ClockType;

// ------------------------------
// Time structs
// ------------------------------

typedef struct {
    i16 west_offset_minutes;
    i16 dst_time_offset_minutes;
    u16 dst_time_start_seconds;
    u16 dst_time_end_seconds;
} Timezone;

typedef struct {
    u64 seconds;
    u64 remainder;
} TimeVal;

#endif  // LIBS_LIBC_SRC_INCLUDE_SYS_TIME_H_
