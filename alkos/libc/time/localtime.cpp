#include <assert.h>
#include <sys/calls.h>
#include <time.h>
#include <extensions/time.hpp>

// ------------------------------
// Constants
// ------------------------------

static constexpr u64 kLeap30Posix = 30 * kSecondsInUsualYear + (28 / 4) * kSecondsInDay;

// ------------------------------
// static functions
// ------------------------------

// ------------------------------
// Implementation
// ------------------------------

tm *localtime_r(const time_t *timer, tm *result)
{
    u64 time_left = *timer;

    /* add local time offset */
    // time_left += static_cast<i64>(__GetLocalTimezoneOffsetNs() / kNanosInSecond);

    const u64 years = time_left >= kLeap30Posix ? CalculateYears30MoreWLeaps(time_left)
                                                : CalculateYears30LessWLeaps(time_left);
    return {};
}

tm *localtime(const time_t *timer)
{
    static tm buffer;
    return localtime_r(timer, &buffer);
}
