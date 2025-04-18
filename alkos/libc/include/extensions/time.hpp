#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_TIME_HPP_

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

static constexpr Timezone kUtcTimezone = {
    .west_offset_minutes     = 0,
    .dst_time_offset_minutes = 0,
    .dst_time_start_seconds  = static_cast<u16>(-1),
    .dst_time_end_seconds    = static_cast<u16>(-1),
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
    const i64 total_days = GetWeekdayJan1(SumUpDays(time)) + SumYearDays(time) - 1;
    return total_days % 7;
}

NODISCARD bool ValidateTm(const tm &time_ptr);

/* [years, time] */
NODISCARD std::tuple<u64, u64> CalculateYears30LessWLeaps(u64 time);

/* [years, time] */
NODISCARD std::tuple<u64, u64> CalculateYears30MoreWLeaps(u64 time);

NODISCARD FAST_CALL std::tuple<u64, u64> CalculateYearsFromPosix(const u64 time)
{
    return time >= kFirst30PosixYears ? CalculateYears30MoreWLeaps(time)
                                      : CalculateYears30LessWLeaps(time);
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
NODISCARD std::tuple<u64, u64> CalculateMonthAndDaysFromPosix(u64 days, bool is_leap_year);

NODISCARD u64 GetDSTOffset(u64 time, const timezone &tz);

tm *ConvertFromPosixToTm(time_t timer, tm &result, const timezone &tz);

NODISCARD FAST_CALL time_t MkTimeFromTimeZone(tm &time_ptr, const timezone &time_zone)
{
    const time_t t = ConvertDateTimeToPosix(time_ptr, time_zone);

    if (t == kConversionFailed) {
        errno = EOVERFLOW;
        return kMktimeFailed;
    }

    ConvertFromPosixToTm(t, time_ptr, time_zone);
    ASSERT_EQ(ConvertDateTimeToPosix(time_ptr, time_zone), t);

    return t;
}

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
