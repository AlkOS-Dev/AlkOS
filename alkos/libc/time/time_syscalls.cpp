#include <assert.h>
#include <sys/calls/time.h>
#include <todo.h>
#include <extensions/time.hpp>

#ifdef __ALKOS_KERNEL__

#include <modules/timing.hpp>

// ----------------------------------------
// Internal kernel use implementation
// ----------------------------------------

void GetDayTimeSysCall(TimerVal* time, Timezone* time_zone)
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

#endif  // __ALKOS_KERNEL__
