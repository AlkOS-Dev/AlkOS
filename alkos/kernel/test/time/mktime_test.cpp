/* internal includes */
#include <assert.h>
#include <memory.h>
#include <time.h>
#include <test_module/test.hpp>

TODO_TIMEZONES
/* TODO: Replace with changeable timezones when implemented */

class MkTimeTest : public TestGroupBase
{
    protected:
    static constexpr size_t kBufferSize = 128;
    char buffer_[kBufferSize]{};

    struct tm CreateTimeInfo(
        const int year, const int month, const int day, const int hour = 0, const int min = 0,
        const int sec = 0, const int isdst = 0
    )
    {
        struct tm timeinfo;

        timeinfo.tm_year  = year - 1900;
        timeinfo.tm_mon   = month - 1;
        timeinfo.tm_mday  = day;
        timeinfo.tm_hour  = hour;
        timeinfo.tm_min   = min;
        timeinfo.tm_sec   = sec;
        timeinfo.tm_isdst = isdst;

        return timeinfo;
    }
};

#define VERIFY_MKTIME(                                                                      \
    input_tm, expected_timestamp, exp_year, exp_month, exp_day, exp_hour, exp_min, exp_sec, \
    exp_wday                                                                                \
)                                                                                           \
    {                                                                                       \
        struct tm timeinfo = input_tm;                                                      \
        time_t result      = mktime(&timeinfo);                                             \
        EXPECT_EQ(expected_timestamp, result);                                              \
        EXPECT_EQ((exp_year) - 1900, timeinfo.tm_year);                                     \
        EXPECT_EQ((exp_month) - 1, timeinfo.tm_mon);                                        \
        EXPECT_EQ(exp_day, timeinfo.tm_mday);                                               \
        EXPECT_EQ(exp_hour, timeinfo.tm_hour);                                              \
        EXPECT_EQ(exp_min, timeinfo.tm_min);                                                \
        EXPECT_EQ(exp_sec, timeinfo.tm_sec);                                                \
        EXPECT_EQ(exp_wday, timeinfo.tm_wday);                                              \
    }

// ------------------------------
// Basic MkTime Tests
// ------------------------------

TEST_F(MkTimeTest, BasicMkTimeConversion)
{
    struct tm t1 = CreateTimeInfo(2024, 1, 15, 13, 50, 45, 0);
    static constexpr time_t expected =
        1705323045;  // 2024-01-15 12:50:45 UTC (assuming local is UTC+1)

    VERIFY_MKTIME(t1, expected, 2024, 1, 15, 13, 50, 45, 1);  // Monday

    TODO_TIMEZONES
    struct tm t2                            = CreateTimeInfo(2024, 7, 15, 18, 57, 25, 0);
    static constexpr time_t summer_expected = 1721066245;  // 2024-07-15 17:57:25 UTC TODO: no dst

    VERIFY_MKTIME(t2, summer_expected, 2024, 7, 15, 18, 57, 25, 1);  // Monday

    struct tm t3                           = CreateTimeInfo(2025, 1, 1, 12, 0, 0);
    static constexpr time_t month_expected = 1735729200;  // 2025-01-01 11:00:00 UTC

    VERIFY_MKTIME(t3, month_expected, 2025, 1, 1, 12, 0, 0, 3);  // Wednesday
}

// ------------------------------
// Normalization Tests
// ------------------------------

TEST_F(MkTimeTest, NormalizesTimeComponents)
{
    // Test with values that need normalization
    struct tm overflowed = CreateTimeInfo(2024, 1, 15, 25, 70, 75);

    // Should normalize to 2024-01-16 02:11:15
    static constexpr time_t expected = 1705367475;  // Expected normalized timestamp

    VERIFY_MKTIME(overflowed, expected, 2024, 1, 16, 2, 11, 15, 2);  // Tuesday

    // Test with month overflow
    struct tm month_overflow               = CreateTimeInfo(2024, 13, 1, 12, 0, 0);
    static constexpr time_t month_expected = 1735729200;  // 2025-01-01 11:00:00 UTC

    VERIFY_MKTIME(month_overflow, month_expected, 2025, 1, 1, 12, 0, 0, 3);  // Wednesday
}

// ------------------------------
// Special Cases Tests
// ------------------------------

TEST_F(MkTimeTest, LeapYearHandling)
{
    // February 29, 2024 12:00:00 (local time)
    struct tm leap_day   = CreateTimeInfo(2024, 2, 29, 12, 0, 0);
    time_t leap_expected = 1709204400;  // 2024-02-29 11:00:00 UTC

    VERIFY_MKTIME(leap_day, leap_expected, 2024, 2, 29, 12, 0, 0, 4);  // Thursday

    // February 29, 2023 12:00:00 (should normalize to March 1st)
    struct tm non_leap_day   = CreateTimeInfo(2023, 2, 29, 12, 0, 0);
    time_t non_leap_expected = 1677668400;  // 2023-03-01 11:00:00 UTC

    VERIFY_MKTIME(non_leap_day, non_leap_expected, 2023, 3, 1, 12, 0, 0, 3);  // Wednesday
}

TEST_F(MkTimeTest, OutOfRangeHandling)
{
    TODO_TIMEZONES
    struct tm invalid_day       = CreateTimeInfo(2024, 4, 31, 11, 0, 0);
    time_t invalid_day_expected = 1714557600;  // 2024-05-01 10:00:00 UTC

    VERIFY_MKTIME(invalid_day, invalid_day_expected, 2024, 5, 1, 11, 0, 0, 3);  // Wednesday

    struct tm negative_values = CreateTimeInfo(2024, 5, 14, -10, -30, -15);
    time_t negative_expected  = 1715603385;  // 2024-05-13 12:29:45 UTC

    VERIFY_MKTIME(negative_values, negative_expected, 2024, 5, 13, 13, 29, 45, 1);  // Monday
}

TEST_F(MkTimeTest, YearDayCalculation)
{
    TODO_TIMEZONES
    struct tm time_info = CreateTimeInfo(2024, 7, 15, 16, 40, 0);
    time_t expected     = 1721058000;  // 2024-07-15 15:40:00 UTC

    time_t result = mktime(&time_info);
    EXPECT_EQ(expected, result);

    // July 15 is the 196th day of 2024 (leap year)
    EXPECT_EQ(196, time_info.tm_yday);

    struct tm time_info2 = CreateTimeInfo(2023, 3, 1, 12, 0, 0);
    mktime(&time_info2);

    // March 1 is the 59th day of 2023 (non-leap year)
    EXPECT_EQ(59, time_info2.tm_yday);

    struct tm time_info3 = CreateTimeInfo(2024, 3, 1, 12, 0, 0);
    mktime(&time_info3);

    // March 1 is the 60th day of 2024 (leap year)
    EXPECT_EQ(60, time_info3.tm_yday);
}
