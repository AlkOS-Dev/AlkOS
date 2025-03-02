#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <extensions/time.hpp>

// ------------------------------
// Constants
// ------------------------------

static constexpr uint64_t kConversionFailed = static_cast<uint64_t>(-1);

static uint16_t kDaysInMonth[2][13]{
    /* Normal Year */
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    /* Leap Year */
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};

// ------------------------------
// static functions
// ------------------------------

static uint64_t ConvertDateTime(const tm &date_time)
{
    const int64_t is_month_negative = date_time.tm_mon < 0;

    /* when month is negative, we goes backward */
    const int64_t month_remainder = (date_time.tm_mon % 12) + is_month_negative * 12;
    const int64_t month_years     = date_time.tm_mon / 12;
    const int64_t years           = date_time.tm_year + month_years;

    /* we should not count the current month */
    const int64_t days = kDaysInMonth[IsTmYearLeap(years)][month_remainder];

    int64_t time = date_time.tm_sec;
    time += static_cast<int64_t>(date_time.tm_min) * kSecondsInMinute;
    time += static_cast<int64_t>(date_time.tm_hour) * kSecondsInHour;
    time += static_cast<int64_t>(date_time.tm_mday - 1) * kSecondsInDay;
    time += static_cast<int64_t>(days) * kSecondsInDay;
    time += static_cast<int64_t>(years) * kSecondsInUsualYear;

    /* adjust by leap years */
    time += years / 4 * kSecondsInDay;
    time -= years / 100 * kSecondsInDay;
    time += years / 400 * kSecondsInDay;

    /* adjust to fit in posix */
    time -= kPosixEpochTmSecondDiff;

    /* adjust by timezone */
    // time -= static_cast<int64_t>(__GetLocalTimezoneOffsetNs() / kNanosInSecond);
    //
    // if (date_time.tm_isdst > 0) {
    //     time -= static_cast<int64_t>(__GetDstTimezoneOffsetNs() / kNanosInSecond);
    // }

    if (date_time.tm_isdst < 0) {
        /* TODO: here we should try to determine if DST is in effect */
        TODO_BY_THE_END_OF_MILESTONE1
        assert(false && "DST guess is not supported yet");
    }

    if (time < 0) {
        return kConversionFailed;
    }

    return time;
}

// ------------------------------
// Implementation
// ------------------------------

time_t mktime(tm *time_ptr)
{
    const time_t t = ConvertDateTime(*time_ptr);

    if (t == kConversionFailed) {
        errno = EOVERFLOW;
        return kMktimeFailed;
    }

    /* TODO: use localtime to adjust tm structure */
    localtime_r(&t, time_ptr);

    return t;
}
