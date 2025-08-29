#ifndef ALKOS_LIBC_INCLUDE_SYS_CALLS_TIME_H_
#define ALKOS_LIBC_INCLUDE_SYS_CALLS_TIME_H_

#include "platform.h"
#include "defines.h"
#include "sys/time.h"

// ------------------------------
// System calls
// ------------------------------

BEGIN_DECL_C

inline void GetClockValueSysCall(ClockType type, TimeVal* time, Timezone* time_zone) {
    __platform_get_clock_value(type, time, time_zone);
}

inline void GetTimezoneSysCall(Timezone* time_zone) {
    __platform_get_timezone(time_zone);
}

inline u64 GetClockTicksInSecondSysCall(ClockType type) {
    return __platform_get_clock_ticks_in_second(type);
}

END_DECL_C

// ------------------------------
// cpp extensions
// ------------------------------

#ifdef __cplusplus

#include <extensions/tuple.hpp>

WRAP_CALL Timezone GetTimezoneSysCall()
{
    Timezone time_zone;
    GetTimezoneSysCall(&time_zone);
    return time_zone;
}

WRAP_CALL TimeVal GetClockValueSysCall(const ClockType type)
{
    TimeVal time;
    Timezone time_zone;
    GetClockValueSysCall(type, &time, &time_zone);
    return time;
}

WRAP_CALL std::tuple<TimeVal, Timezone> GetClockValueTimezoneSysCall(const ClockType type)
{
    TimeVal time;
    Timezone time_zone;
    GetClockValueSysCall(type, &time, &time_zone);
    return {time, time_zone};
}

#endif  // __cplusplus

#endif  // ALKOS_LIBC_INCLUDE_SYS_CALLS_TIME_H_
