#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_TIME_HPP_

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <extensions/debug.hpp>
#include <extensions/defines.hpp>
#include <extensions/time.hpp>
#include <extensions/types.hpp>

// ------------------------------
// Time constants
// ------------------------------

/* usual time constants */
static constexpr i64 kTmBaseYear    = 1900;
static constexpr i64 kTmBaseYearMod = kTmBaseYear % 400;

static constexpr i64 kHoursInDay = 24;

static constexpr i64 kMinutesInHour = 60;

static constexpr i64 kSecondsInMinute    = 60;
static constexpr i64 kSecondsInHour      = kSecondsInMinute * 60;
static constexpr i64 kSecondsInDay       = kSecondsInHour * 24;
static constexpr i64 kSecondsInUsualYear = kSecondsInDay * 365;

static constexpr u64 kNanosInSecond = 1'000'000'000;

/* posix epoch */
static constexpr i64 kPosixEpoch = 1970;

static constexpr u64 kPosixYearsToFirstLeap    = 2;
static constexpr u64 kPosixYearsToFirstLeap100 = 30;
static constexpr u64 kPosixYearsToFirstLeap400 = 30;

static constexpr u64 kPosixEpochTmSecondDiff = (kPosixEpoch - kTmBaseYear) * kSecondsInUsualYear +
                                               ((kPosixEpoch - kTmBaseYear) / 4) * kSecondsInDay;

static constexpr u16 kDaysInMonth[2][13]{
    /* Normal Year */
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    /* Leap Year */
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};

// ------------------------------
// mktime constants
// ------------------------------

static constexpr time_t kMktimeFailed  = static_cast<time_t>(-1);
static constexpr u64 kConversionFailed = static_cast<u64>(-1);

// ------------------------------
// Functions
// ------------------------------

FAST_CALL bool IsLeapYear(const i64 year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

WRAP_CALL bool IsTmYearLeap(const i64 year) { return IsLeapYear(year + kTmBaseYear); }

WRAP_CALL bool IsTmYearLeap(const tm &time) { return IsLeapYear(time.tm_year + kTmBaseYear); }

NODISCARD u64 ConvertDateTimeToSeconds(const tm &date_time, const timezone &time_zone);

FAST_CALL i64 SumUpDays(const i64 year)
{
    const i64 years_adjusted = year - 1;

    return years_adjusted * 365 + years_adjusted / 4 - years_adjusted / 100 + years_adjusted / 400;
}

WRAP_CALL i64 SumUpDays(const tm &time_ptr) { return SumUpDays(time_ptr.tm_year + kTmBaseYear); }

/**
 * @note 0 = Sunday, 1 = Monday, ..., 6 = Saturday
 */
FAST_CALL i64 GetWeekdayJan1(const i64 days_since_century) { return (days_since_century + 1) % 7; }

/**
 * @note: Includes the current day
 */
FAST_CALL i64 SumYearDays(const tm &time_ptr)
{
    const bool is_leap = IsTmYearLeap(time_ptr.tm_year);
    return kDaysInMonth[is_leap][time_ptr.tm_mon] + time_ptr.tm_mday;
}

/**
 * @note 0 = Sunday, 1 = Monday, ..., 6 = Saturday
 */
FAST_CALL i64 CalculateDayOfWeek(const tm &time)
{
    const i64 total_days = GetWeekdayJan1(SumUpDays(time)) + SumYearDays(time);
    return total_days % 7;
}

NODISCARD bool ValidateTm(const tm &time_ptr);

/* Posix time helpers */
FAST_CALL u64 CalculateYears30LessWLeaps(const u64 time_left) { return {}; }

FAST_CALL u64 CalculateYears30MoreWLeaps(const u64 time_left)
{
    static constexpr u64 kDown = 400 * kSecondsInUsualYear + 97 * kSecondsInDay;

    const u64 up       = time_left + 110 * kSecondsInDay;
    const u64 estimate = 400 * (kSecondsInUsualYear + kSecondsInDay);

    const u64 low  = (up - estimate) / kDown;
    const u64 high = up / kDown;

    return {};
}

NODISCARD FAST_CALL i64 CalculateMondayBasedWeek(const tm &time)
{
    const i64 jan1_weekday              = GetWeekdayJan1(SumUpDays(time));
    const i64 days                      = SumYearDays(time) - 1;
    const i64 monday_based_jan1_weekday = jan1_weekday == 1   ? 7
                                          : jan1_weekday == 0 ? 6
                                                              : jan1_weekday - 1;

    TRACE_INFO("jan1_weekday = %lu", jan1_weekday);
    TRACE_INFO("Jan1_weekday_monday_based: %lu", monday_based_jan1_weekday);

    return (days + monday_based_jan1_weekday) / 7;
}

NODISCARD FAST_CALL i64 CalculateSundayBasedWeek(const tm &time)
{
    const i64 jan1_weekday         = GetWeekdayJan1(SumUpDays(time));
    const i64 days                 = SumYearDays(time) - 1;
    const i64 sunday_based_weekday = jan1_weekday == 0 ? 7 : jan1_weekday;

    return (days + sunday_based_weekday) / 7;
}

NODISCARD i64 CalculateIsoBasedWeek(const tm &time);

NODISCARD i64 CalculateIsoBasedYear(const tm &time);

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
