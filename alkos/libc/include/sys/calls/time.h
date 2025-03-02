#ifndef LIBC_INCLUDE_SYS_CALLS_TIME_H_
#define LIBC_INCLUDE_SYS_CALLS_TIME_H_

#include <defines.h>
#include <sys/time.h>

BEGIN_DECL_C
void GetDayTimeSysCall(TimerVal* time, Timezone* time_zone);
void GetTimezoneSysCall(Timezone* time_zone);
END_DECL_C

#ifdef __cplusplus

#include <extensions/tuple.hpp>

WRAP_CALL Timezone GetTimezoneSysCall()
{
    Timezone time_zone;
    GetTimezoneSysCall(&time_zone);
    return time_zone;
}

WRAP_CALL TimerVal GetDayTimeSysCall()
{
    TimerVal time;
    Timezone time_zone;
    GetDayTimeSysCall(&time, &time_zone);
    return time;
}

WRAP_CALL std::tuple<TimerVal, Timezone> GetDayTimeTimezoneSysCall()
{
    TimerVal time;
    Timezone time_zone;
    GetDayTimeSysCall(&time, &time_zone);
    return {time, time_zone};
}

#endif  // __cplusplus

#endif  // LIBC_INCLUDE_SYS_CALLS_TIME_H_
