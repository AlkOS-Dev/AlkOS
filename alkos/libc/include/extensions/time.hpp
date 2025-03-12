#ifndef LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
#define LIBC_INCLUDE_EXTENSIONS_TIME_HPP_

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <extensions/debug.hpp>
#include <extensions/defines.hpp>
#include <extensions/time.hpp>
#include <extensions/tuple.hpp>
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

static constexpr u64 kFirst30PosixYears = 30 * kSecondsInUsualYear + (28 / 4) * kSecondsInDay;

static constexpr u64 kPosixToTmYearDiff = kPosixEpoch - kTmBaseYear;

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

FAST_CALL constexpr bool IsLeapYear(const i64 year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

WRAP_CALL constexpr bool IsTmYearLeap(const i64 year) { return IsLeapYear(year + kTmBaseYear); }

WRAP_CALL constexpr bool IsTmYearLeap(const tm &time)
{
    return IsLeapYear(time.tm_year + kTmBaseYear);
}

NODISCARD u64 ConvertDateTimeToPosix(const tm &date_time, const timezone &time_zone);

FAST_CALL i64 constexpr SumUpDays(const i64 year)
{
    const i64 years_adjusted = year - 1;

    return years_adjusted * 365 + years_adjusted / 4 - years_adjusted / 100 + years_adjusted / 400;
}

WRAP_CALL constexpr i64 SumUpDays(const tm &time_ptr)
{
    return SumUpDays(time_ptr.tm_year + kTmBaseYear);
}

/**
 * @note 0 = Sunday, 1 = Monday, ..., 6 = Saturday
 */
FAST_CALL constexpr i64 GetWeekdayJan1(const i64 days_since_century)
{
    return (days_since_century + 1) % 7;
}

/**
 * @note: Includes the current day
 */
FAST_CALL constexpr i64 SumYearDays(const tm &time_ptr)
{
    const bool is_leap = IsTmYearLeap(time_ptr.tm_year);
    return kDaysInMonth[is_leap][time_ptr.tm_mon] + time_ptr.tm_mday;
}

/**
 * @note 0 = Sunday, 1 = Monday, ..., 6 = Saturday
 */
FAST_CALL constexpr i64 CalculateDayOfWeek(const tm &time)
{
    const i64 total_days = GetWeekdayJan1(SumUpDays(time)) + SumYearDays(time);
    return total_days % 7;
}

NODISCARD bool ValidateTm(const tm &time_ptr);

/* [years, time_left] */
NODISCARD FAST_CALL constexpr std::tuple<u64, u64> CalculateYears30LessWLeaps(const u64 time_left)
{
    i64 local_time_left;
    i64 years                       = time_left / kSecondsInUsualYear;
    [[maybe_unused]] i64 iterations = 0;

    do {
        assert(iterations++ < 2);

        local_time_left = static_cast<i64>(time_left);

        local_time_left -= years * kSecondsInUsualYear;

        /* Adjust by leap years */
        local_time_left -= years > 2 ? (years - 2) / 4 * kSecondsInDay : 0;

        /* prepare for next iteration */
        years -= 1;
    } while (local_time_left < 0);

    return {years + 1, static_cast<u64>(local_time_left)};
}

/* [years, time_left] */
NODISCARD FAST_CALL constexpr std::tuple<u64, u64> CalculateYears30MoreWLeaps(const u64 time_left)
{
    i64 local_time_left;
    i64 years                       = (time_left - 30) / kSecondsInUsualYear;
    [[maybe_unused]] i64 iterations = 0;

    do {
        assert(iterations++ < 5);

        local_time_left = static_cast<i64>(time_left);

        local_time_left -= years * kSecondsInUsualYear;

        /* Adjust by leap years div 4 */
        local_time_left -= years / 4 * kSecondsInDay;

        /* Adjust by leap years div 100 */
        local_time_left += years / 100 * kSecondsInDay;

        /* Adjust by leap years div 400 */
        local_time_left -= years / 400 * kSecondsInDay;
        local_time_left -= kSecondsInDay; /* We start from year 2000 */

        /* prepare for next iteration */
        years -= 1;
    } while (local_time_left < 0);

    return {years + 1 + 30, static_cast<u64>(local_time_left)};
}

NODISCARD FAST_CALL std::tuple<u64, u64> CalculateYearsFromPosix(const u64 time_left)
{
    return time_left >= kFirst30PosixYears ? CalculateYears30MoreWLeaps(time_left)
                                           : CalculateYears30LessWLeaps(time_left);
}

NODISCARD FAST_CALL i64 CalculateMondayBasedWeek(const tm &time)
{
    const i64 jan1_weekday              = GetWeekdayJan1(SumUpDays(time));
    const i64 days                      = SumYearDays(time) - 1;
    const i64 monday_based_jan1_weekday = jan1_weekday == 1   ? 7
                                          : jan1_weekday == 0 ? 6
                                                              : jan1_weekday - 1;

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

/* [month, day] */
NODISCARD FAST_CALL std::tuple<u64, u64> CalculateMonthAndDaysFromPosix(
    const u64 days, const bool is_leap_year
)
{
    ASSERT_LT(days, 366_u64);

    for (size_t idx = 1; idx < 12; ++idx) {
        if (days < kDaysInMonth[is_leap_year][idx]) {
            return {idx, days - kDaysInMonth[is_leap_year][idx - 1] + 1};
        }
    }

    return {12, days - kDaysInMonth[is_leap_year][11] + 1};
}

tm *localtime_r(const time_t *timer, tm *result, const timezone &tz);

#endif  // LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
