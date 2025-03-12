#include <assert.h>
#include <sys/calls.h>
#include <time.h>
#include <extensions/time.hpp>

// ------------------------------
// Constants
// ------------------------------

// ------------------------------
// static functions
// ------------------------------

// ------------------------------
// Implementation
// ------------------------------

tm *localtime_r(const time_t *timer, tm *result)
{
    const auto time_zone = GetTimezoneSysCall();
    u64 time_left        = *timer;

    /* add local time offset */
    time_left += time_zone.west_offset_minutes * kSecondsInMinute;

    const auto [years, time_left_after_years] = CalculateYearsFromPosix(time_left);

    /* Apply years */
    time_left       = time_left_after_years;
    result->tm_year = static_cast<int>(kPosixToTmYearDiff + years);

    /* Check if DST is in effect */
    if (time_zone.dst_time_start_seconds < time_zone.dst_time_end_seconds) {
        /* Continuous DST */

        if (time_left >= time_zone.dst_time_start_seconds &&
            time_left < time_zone.dst_time_end_seconds) {
            result->tm_isdst = 1;
            time_left += time_zone.dst_time_offset_minutes * kSecondsInMinute;
        } else {
            result->tm_isdst = 0;
        }
    } else {
        /* Spans over the year */

        if (time_left >= time_zone.dst_time_start_seconds ||
            time_left < time_zone.dst_time_end_seconds) {
            result->tm_isdst = 1;
            time_left += time_zone.dst_time_offset_minutes * kSecondsInMinute;
        } else {
            result->tm_isdst = 0;
        }
    }

    /* Apply days */
    const u64 days = time_left / kSecondsInDay;
    time_left -= days * kSecondsInDay;
    result->tm_yday = static_cast<int>(days);

    /* Apply hours */
    const u64 hours = time_left / kSecondsInHour;
    time_left -= hours * kSecondsInHour;
    result->tm_hour = static_cast<int>(hours);

    /* Apply minutes */
    const u64 minutes = time_left / kSecondsInMinute;
    time_left -= minutes * kSecondsInMinute;
    result->tm_min = static_cast<int>(minutes);

    /* Apply seconds */
    result->tm_sec = static_cast<int>(time_left);

    /* Apply month and days */
    const auto [month, day] = CalculateMonthAndDaysFromPosix(days, IsTmYearLeap(*result));
    result->tm_mon          = static_cast<int>(month - 1);
    result->tm_mday         = static_cast<int>(day);
    result->tm_wday         = static_cast<int>(CalculateSundayBasedWeek(*result));

    return result;
}

tm *localtime(const time_t *timer)
{
    static tm buffer;
    return localtime_r(timer, &buffer);
}
