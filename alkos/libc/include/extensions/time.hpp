#ifndef LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
#define LIBC_INCLUDE_EXTENSIONS_TIME_HPP_

#include <stdint.h>
#include <time.h>

// ------------------------------
// Time constants
// ------------------------------

/* usual time constants */
static constexpr int64_t kTmBaseYear    = 1900;
static constexpr int64_t kTmBaseYearMod = kTmBaseYear % 400;

static constexpr int64_t kSecondsInMinute    = 60;
static constexpr int64_t kSecondsInHour      = kSecondsInMinute * 60;
static constexpr int64_t kSecondsInDay       = kSecondsInHour * 24;
static constexpr int64_t kSecondsInUsualYear = kSecondsInDay * 365;

/* posix epoch */
static constexpr int64_t kPosixEpoch = 1970;

static constexpr uint64_t kPosixEpochTmSecondDiff =
    (kPosixEpoch - kTmBaseYear) * kSecondsInUsualYear +
    ((kPosixEpoch - kTmBaseYear) / 4) * kSecondsInDay;

// ------------------------------
// mktime constants
// ------------------------------

static constexpr time_t kMktimeFailed = static_cast<time_t>(-1);

// ------------------------------
// Functions
// ------------------------------

FAST_CALL bool IsLeapYear(const int64_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

WRAP_CALL bool IsTmYearLeap(const int64_t year) { return IsLeapYear(year + kTmBaseYear); }

#endif  // LIBC_INCLUDE_EXTENSIONS_TIME_HPP_
