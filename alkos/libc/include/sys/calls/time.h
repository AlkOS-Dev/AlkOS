#ifndef LIBC_INCLUDE_SYS_CALLS_TIME_H_
#define LIBC_INCLUDE_SYS_CALLS_TIME_H_

#include <defines.h>
#include <sys/time.h>

// ------------------------------
// System calls
// ------------------------------

BEGIN_DECL_C

void GetClockValueSysCall(ClockType type, TimeVal* time, Timezone* time_zone);

void GetTimezoneSysCall(Timezone* time_zone);

u64 GetClockTicksInSecondSysCall(ClockType type);

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

#endif  // LIBC_INCLUDE_SYS_CALLS_TIME_H_
