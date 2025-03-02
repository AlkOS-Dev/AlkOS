#include <assert.h>
#include <sys/calls.h>
#include <time.h>
#include <extensions/time.hpp>

// ------------------------------
// Constants
// ------------------------------

static constexpr uint64_t kLeap30Posix = 30 * kSecondsInUsualYear + (28 / 4) * kSecondsInDay;

// ------------------------------
// static functions
// ------------------------------

// ------------------------------
// Implementation
// ------------------------------

tm *localtime_r(const time_t *timer, tm *result)
{
    uint64_t time_left = *timer;

    /* add local time offset */
    // time_left += static_cast<int64_t>(__GetLocalTimezoneOffsetNs() / kNanosInSecond);

    const uint64_t years = time_left >= kLeap30Posix ? CalculateYears30MoreWLeaps(time_left)
                                                     : CalculateYears30LessWLeaps(time_left);

    return {};
}

tm *localtime(const time_t *timer)
{
    static tm buffer;
    return localtime_r(timer, &buffer);
}
