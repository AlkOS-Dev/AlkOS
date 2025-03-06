#ifndef LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
#define LIBC_INCLUDE_EXTENSIONS_TIME_HPP_

#include <stdint.h>
#include <time.h>

#include <extensions/time.hpp>

// ------------------------------
// Time constants
// ------------------------------

/* usual time constants */
static constexpr i64 kTmBaseYear    = 1900;
static constexpr i64 kTmBaseYearMod = kTmBaseYear % 400;

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

// ------------------------------
// mktime constants
// ------------------------------

static constexpr time_t kMktimeFailed = static_cast<time_t>(-1);

// ------------------------------
// Functions
// ------------------------------

FAST_CALL bool IsLeapYear(const i64 year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

WRAP_CALL bool IsTmYearLeap(const i64 year) { return IsLeapYear(year + kTmBaseYear); }

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

#endif  // LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
