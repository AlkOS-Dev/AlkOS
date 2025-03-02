#include <assert.h>
#include <sys/calls/time.h>
#include <todo.h>
#include <extensions/time.hpp>

#ifdef __ALKOS_KERNEL__

// ----------------------------------------
// Internal kernel use implementation
// ----------------------------------------

void GetDayTimeSysCall(TimerVal* time, Timezone* time_zone)
{
    assert(time != nullptr && time_zone != nullptr);

    GetTimezoneSysCall(time_zone);
}

void GetTimezoneSysCall(Timezone* time_zone)
{
    assert(time_zone != nullptr);

    TODO_TIMEZONES

    /* Hard coded Poland */
    static constexpr uint64_t kPolandOffset = 1;
    time_zone->west_offset_seconds          = kPolandOffset * kSecondsInHour;

    /* TODO: temporarily disable DST */
    time_zone->dst_time_offset_seconds = 0;
}

#else

// ----------------------------------------
// Actual system calls implementation
// ----------------------------------------

TODO_USERSPACE

#endif  // __ALKOS_KERNEL__
