#ifndef LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
#define LIBC_INCLUDE_EXTENSIONS_TIME_HPP_

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <extensions/defines.hpp>

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

NODISCARD int64_t CalculateMondayBasedWeek(const tm &time);

NODISCARD int64_t CalculateSundayBasedWeek(const tm &time);

#endif  // LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
