/* internal includes */
#include <assert.h>
#include <time.h>

double difftime(const time_t time_end, const time_t time_beg)
{
    return static_cast<double>(time_end - time_beg);
}

time_t time(time_t *arg)
{
    assert(false && "Not implemented!");
    TODO_CLOCKS
    return time_t{};
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
