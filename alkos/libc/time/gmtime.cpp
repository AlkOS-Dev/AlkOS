#include <assert.h>
#include <time.h>
#include <extensions/time.hpp>

struct tm *gmtime_r(const time_t *timer, struct tm *result)
{
    return ConvertFromPosixToTm(*timer, *result, kUtcTimezone);
}

struct tm *gmtime(const time_t *timer)
{
    static tm buffer;
    return gmtime_r(timer, &buffer);
}
