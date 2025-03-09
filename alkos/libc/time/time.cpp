/* internal includes */
#include <assert.h>
#include <time.h>

#include <sys/calls.h>
#include <sys/time.h>

double difftime(const time_t time_end, const time_t time_beg)
{
    return static_cast<double>(time_end - time_beg);
}

time_t time(time_t *arg)
{
    time_val tv;
    GetDayTimeSysCall(&tv, nullptr);

    if (arg != nullptr) {
        *arg = static_cast<time_t>(tv.seconds);
    }

    return tv.seconds;
}

clock_t clock()
{
    assert(false && "Not implemented!");
    TODO_CLOCKS
    return clock_t{};
}

int timespec_get(struct timespec *ts, int base)
{
    assert(false && "Not implemented!");
    TODO_CLOCKS
    return int{};
}

int timespec_getres(struct timespec *ts, int base)
{
    assert(false && "Not implemented!");
    TODO_CLOCKS
    return int{};
}
