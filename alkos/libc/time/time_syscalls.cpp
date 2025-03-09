#include <assert.h>
#include <sys/calls/time.h>
#include <todo.h>
#include <extensions/time.hpp>

#ifdef __ALKOS_LIBK__

#include <modules/timing.hpp>

// ----------------------------------------
// Internal kernel use implementation
// ----------------------------------------

void GetDayTimeSysCall(TimeVal* time, Timezone* time_zone)
{
    assert(time != nullptr || time_zone != nullptr);

    if (time_zone != nullptr) {
        GetTimezoneSysCall(time_zone);
    }

    if (time != nullptr) {
        time->seconds     = TimingModule::Get().GetDayTime().GetTime();
        time->nanoseconds = 0;
    }
}

void GetTimezoneSysCall(Timezone* time_zone)
{
    assert(time_zone != nullptr);
    *time_zone = TimingModule::Get().GetDayTime().GetTimezone();
}

#else

// ----------------------------------------
// Actual system calls implementation
// ----------------------------------------

TODO_USERSPACE

#endif  // __ALKOS_LIBK__
