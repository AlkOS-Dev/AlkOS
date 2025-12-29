#include <sys/calls.h>
#include <time.h>
#include <time.hpp>
// ------------------------------
// Implementation
// ------------------------------

tm *localtime_r(const time_t *timer, tm *result)
{
    const auto time_zone = GetTimezoneSysCall();
    return ConvertFromPosixToTm(*timer, *result, time_zone);
}

tm *localtime(const time_t *timer)
{
    static tm buffer;
    return localtime_r(timer, &buffer);
}
