#ifndef LIBC_INCLUDE_SYS_CALLS_TIME_H_
#define LIBC_INCLUDE_SYS_CALLS_TIME_H_

#include <defines.h>
#include <sys/time.h>

BEGIN_DECL_C
void GetDayTimeSysCall(TimerVal* time, Timezone* time_zone);
void GetTimezoneSysCall(Timezone* time_zone);
END_DECL_C

#endif  // LIBC_INCLUDE_SYS_CALLS_TIME_H_
