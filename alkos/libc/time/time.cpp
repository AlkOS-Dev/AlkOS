/* internal includes */
#include <assert.h>
#include <time.h>

#include <sys/calls.h>
#include <sys/time.h>

static constexpr u64 kNSecInSec = 1'000'000'000;

double difftime(const time_t time_end, const time_t time_beg)
{
    return static_cast<double>(time_end - time_beg);
}

time_t time(time_t *arg)
{
    const auto tv = GetClockValueSysCall(ClockType::kTimeUtc);
    return tv.seconds;
}

clock_t clock()
{
    const auto tv = GetClockValueSysCall(ClockType::kProcTime);
    return tv.remainder;
}

int timespec_get(struct timespec *ts, int base)
{
    if (base <= 0 || base >= ClockType::kLastClockType) {
        return 0;
    }

    const auto tv                   = GetClockValueSysCall(static_cast<ClockType>(base));
    const u64 clock_ticks_in_second = GetClockTicksInSecondSysCall(static_cast<ClockType>(base));

    ts->tv_sec  = tv.seconds + (tv.remainder / clock_ticks_in_second);
    ts->tv_nsec = static_cast<long>(
        (kNSecInSec / clock_ticks_in_second) * (tv.remainder % clock_ticks_in_second)
    );

    return base;
}

int timespec_getres(struct timespec *ts, int base)
{
    if (base <= 0 || base >= ClockType::kLastClockType) {
        return 0;
    }

    const u64 clock_ticks_in_second = GetClockTicksInSecondSysCall(static_cast<ClockType>(base));

    if (clock_ticks_in_second == 1) {
        ts->tv_nsec = 0;
        ts->tv_sec  = 1;
    } else {
        ts->tv_sec  = 0;
        ts->tv_nsec = static_cast<long>(kNSecInSec / clock_ticks_in_second);
    }

    return base;
}
