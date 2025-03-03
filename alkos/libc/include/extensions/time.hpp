#ifndef LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
#define LIBC_INCLUDE_EXTENSIONS_TIME_HPP_

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <extensions/defines.hpp>

#ifdef __ALKOS_KERNEL__
#include <extensions/debug.hpp>
#else
#define TRACE_INFO(...)
#endif

// ------------------------------
// Time constants
// ------------------------------

/* usual time constants */
static constexpr int64_t kTmBaseYear    = 1900;
static constexpr int64_t kTmBaseYearMod = kTmBaseYear % 400;

static constexpr int64_t kHoursInDay = 24;

static constexpr int64_t kMinutesInHour = 60;

static constexpr int64_t kSecondsInMinute    = 60;
static constexpr int64_t kSecondsInHour      = kSecondsInMinute * 60;
static constexpr int64_t kSecondsInDay       = kSecondsInHour * 24;
static constexpr int64_t kSecondsInUsualYear = kSecondsInDay * 365;

static constexpr uint64_t kNanosInSecond = 1'000'000'000;

/* posix epoch */
static constexpr int64_t kPosixEpoch = 1970;

static constexpr uint64_t kPosixYearsToFirstLeap    = 2;
static constexpr uint64_t kPosixYearsToFirstLeap100 = 30;
static constexpr uint64_t kPosixYearsToFirstLeap400 = 30;

static constexpr uint64_t kPosixEpochTmSecondDiff =
    (kPosixEpoch - kTmBaseYear) * kSecondsInUsualYear +
    ((kPosixEpoch - kTmBaseYear) / 4) * kSecondsInDay;

static constexpr uint16_t kDaysInMonth[2][13]{
    /* Normal Year */
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    /* Leap Year */
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};

// ------------------------------
// mktime constants
// ------------------------------

static constexpr time_t kMktimeFailed       = static_cast<time_t>(-1);
static constexpr uint64_t kConversionFailed = static_cast<uint64_t>(-1);

// ------------------------------
// Functions
// ------------------------------

FAST_CALL bool IsLeapYear(const int64_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

WRAP_CALL bool IsTmYearLeap(const int64_t year) { return IsLeapYear(year + kTmBaseYear); }

NODISCARD uint64_t ConvertDateTimeToSeconds(const tm &date_time, const timezone &time_zone);

FAST_CALL int64_t SumUpDays_(const int64_t year)
{
    const int64_t years_adjusted = year - 1;

    return years_adjusted * 365 + years_adjusted / 4 - years_adjusted / 100 + years_adjusted / 400;
}

WRAP_CALL int64_t SumUpDays_(const tm &time_ptr)
{
    return SumUpDays_(time_ptr.tm_year + kTmBaseYear);
}

FAST_CALL int64_t GetWeekdayJan1_(const int64_t days_since_century)
{
    return (days_since_century + 1) % 7;
}

/**
 * @note: Includes the current day
 */
FAST_CALL int64_t SumYearDays_(const tm &time_ptr)
{
    const bool is_leap = IsTmYearLeap(time_ptr.tm_year);
    return kDaysInMonth[is_leap][time_ptr.tm_mon] + time_ptr.tm_mday;
}

FAST_CALL int64_t CalculateDayOfWeek(const tm &time)
{
    const int64_t total_days = SumUpDays_(time) + SumYearDays_(time) - 1;
    return total_days % 7;
}

/* Posix time helpers */
FAST_CALL uint64_t CalculateYears30LessWLeaps(const uint64_t time_left) { return {}; }

FAST_CALL uint64_t CalculateYears30MoreWLeaps(const uint64_t time_left)
{
    static constexpr uint64_t kDown = 400 * kSecondsInUsualYear + 97 * kSecondsInDay;

    const uint64_t up       = time_left + 110 * kSecondsInDay;
    const uint64_t estimate = 400 * (kSecondsInUsualYear + kSecondsInDay);

    const uint64_t low  = (up - estimate) / kDown;
    const uint64_t high = up / kDown;

    return {};
}

NODISCARD FAST_CALL int64_t CalculateMondayBasedWeek(const tm &time)
{
    const int64_t jan1_weekday              = GetWeekdayJan1_(time.tm_year);
    const int64_t days                      = SumYearDays_(time) - 1;
    const int64_t monday_based_jan1_weekday = jan1_weekday == 1   ? 7
                                              : jan1_weekday == 0 ? 6
                                                                  : jan1_weekday - 1;

    return (days + monday_based_jan1_weekday) / 7;
}

NODISCARD FAST_CALL int64_t CalculateSundayBasedWeek(const tm &time)
{
    const int64_t jan1_weekday         = GetWeekdayJan1_(SumUpDays_(time));
    const int64_t days                 = SumYearDays_(time) - 1;
    const int64_t sunday_based_weekday = jan1_weekday == 0 ? 7 : jan1_weekday;

    return (days + sunday_based_weekday) / 7;
}

NODISCARD int64_t CalculateIsoBasedWeek(const tm &time);

#endif  // LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
