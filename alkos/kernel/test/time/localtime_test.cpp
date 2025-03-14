/* internal includes */
#include <assert.h>
#include <memory.h>
#include <time.h>
#include <extensions/time.hpp>
#include <test_module/test.hpp>

TODO_TIMEZONES
/* TODO: Replace with changeable timezones when implemented */

class TimeTConversionTest : public TestGroupBase
{
    protected:
    static constexpr size_t kBufferSize = 128;
    char buffer_[kBufferSize]{};

    static time_t CreateTimestamp(
        const int year, const int month, const int day, const int hour = 0, const int min = 0,
        const int sec = 0
    )
    {
        struct tm timeinfo{};
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon  = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min  = min;
        timeinfo.tm_sec  = sec;

        TODO_TIMEZONES
        /* TODO: Enable dst guess when possible */
        timeinfo.tm_isdst = 0;

        return MkTimeFromTimeZone(timeinfo, kUtcTimezone);
    }
};

#define VERIFY_LOCALTIME(                                                         \
    timestamp, exp_year, exp_month, exp_day, exp_hour, exp_min, exp_sec, exp_wday \
)                                                                                 \
    {                                                                             \
        struct tm* timeinfo = localtime(&timestamp);                              \
        ASSERT_NOT_NULL(timeinfo);                                                \
        EXPECT_EQ((exp_year) - 1900, timeinfo->tm_year);                          \
        EXPECT_EQ((exp_month) - 1, timeinfo->tm_mon);                             \
        EXPECT_EQ(exp_day, timeinfo->tm_mday);                                    \
        EXPECT_EQ(exp_hour, timeinfo->tm_hour);                                   \
        EXPECT_EQ(exp_min, timeinfo->tm_min);                                     \
        EXPECT_EQ(exp_sec, timeinfo->tm_sec);                                     \
        EXPECT_EQ(exp_wday, timeinfo->tm_wday);                                   \
    }

#define VERIFY_GMTIME(                                                            \
    timestamp, exp_year, exp_month, exp_day, exp_hour, exp_min, exp_sec, exp_wday \
)                                                                                 \
    {                                                                             \
        struct tm* timeinfo = gmtime(&timestamp);                                 \
        ASSERT_NOT_NULL(timeinfo);                                                \
        EXPECT_EQ((exp_year) - 1900, timeinfo->tm_year);                          \
        EXPECT_EQ((exp_month) - 1, timeinfo->tm_mon);                             \
        EXPECT_EQ(exp_day, timeinfo->tm_mday);                                    \
        EXPECT_EQ(exp_hour, timeinfo->tm_hour);                                   \
        EXPECT_EQ(exp_min, timeinfo->tm_min);                                     \
        EXPECT_EQ(exp_sec, timeinfo->tm_sec);                                     \
        EXPECT_EQ(exp_wday, timeinfo->tm_wday);                                   \
    }

TEST_F(TimeTConversionTest, HelpersTest)
{
    /* 1. January 2000 */
    {
        static constexpr u64 t   = kFirst30PosixYears;
        const auto [years, left] = CalculateYearsFromPosix(t);

        EXPECT_EQ(0_u64, left);
        EXPECT_EQ(30_u64, years);
    }

    /* Second before 2k */
    {
        static constexpr u64 t   = kFirst30PosixYears - 1;
        const auto [years, left] = CalculateYearsFromPosix(t);

        EXPECT_EQ(static_cast<u64>(kSecondsInUsualYear - 1), left);
        EXPECT_EQ(29_u64, years);
    }

    /* Second after 2k */
    {
        static constexpr u64 t   = kFirst30PosixYears + 1;
        const auto [years, left] = CalculateYearsFromPosix(t);

        EXPECT_EQ(1_u64, left);
        EXPECT_EQ(30_u64, years);
    }
}

// ------------------------------
// Basic Time Conversion Tests
// ------------------------------

TEST_F(TimeTConversionTest, BasicTimeConversion)
{
    time_t timestamp = CreateTimestamp(2024, 1, 15, 12, 30, 45);
    VERIFY_LOCALTIME(timestamp, 2024, 1, 15, 13, 30, 45, 1);  // Monday
    VERIFY_GMTIME(timestamp, 2024, 1, 15, 12, 30, 45, 1);     // Monday

    timestamp = CreateTimestamp(2024, 7, 15, 12, 30, 45);
    VERIFY_LOCALTIME(timestamp, 2024, 7, 15, 13, 30, 45, 1);  // Monday
    VERIFY_GMTIME(timestamp, 2024, 7, 15, 12, 30, 45, 1);     // Monday
}

// ------------------------------
// Manual POSIX Timestamp Tests
// ------------------------------

TEST_F(TimeTConversionTest, ManualTimestamps)
{
    // 2024-02-15 12:10:45 UTC
    static constexpr time_t feb15_timestamp = 1707999045;
    VERIFY_LOCALTIME(feb15_timestamp, 2024, 2, 15, 13, 10, 45, 4);  // Thursday
    VERIFY_GMTIME(feb15_timestamp, 2024, 2, 15, 12, 10, 45, 4);     // Thursday

    // 2024-06-15 12:10:45 UTC
    static constexpr time_t jun15_timestamp = 1718457045;
    VERIFY_LOCALTIME(jun15_timestamp, 2024, 6, 15, 14, 10, 45, 6);  // Saturday
    VERIFY_GMTIME(jun15_timestamp, 2024, 6, 15, 13, 10, 45, 6);     // Saturday

    //  30 March 2024 22:59:59 UTC
    static constexpr time_t dst_start_utc = 1711839599;
    VERIFY_LOCALTIME(dst_start_utc, 2024, 3, 30, 23, 59, 59, 6);  // Saturday
    VERIFY_GMTIME(dst_start_utc, 2024, 3, 30, 22, 59, 59, 6);     // Saturday

    //  Saturday, 30 March 2024 23:00:00 UTC
    static constexpr time_t after_dst_utc = 1711839600;
    VERIFY_LOCALTIME(after_dst_utc, 2024, 3, 31, 0, 0, 0, 0);  // Sunday
    VERIFY_GMTIME(after_dst_utc, 2024, 3, 30, 23, 0, 0, 6);    // Saturday

    // Monday, 28 October 2024 14:59:59
    static constexpr time_t before_dst_end_utc = 1730127599;
    VERIFY_LOCALTIME(before_dst_end_utc, 2024, 10, 28, 15, 59, 59, 1);  // Monday
    VERIFY_GMTIME(before_dst_end_utc, 2024, 10, 28, 14, 59, 59, 1);     // Monday

    // 2024-10-27 01:30:00 UTC
    static constexpr time_t after_dst_end_utc = 1730129400;
    VERIFY_LOCALTIME(after_dst_end_utc, 2024, 10, 28, 16, 30, 0, 1);  // Monday
    VERIFY_GMTIME(after_dst_end_utc, 2024, 10, 28, 15, 30, 0, 1);     // Monday

    static constexpr time_t first_day_2k25 = 1735729200;  // 2025-01-01 11:00:00 UTC
    //    VERIFY_LOCALTIME(first_day_2k25, 2025, 1, 1, 12, 0, 0, 3);  // Wednesday
    VERIFY_GMTIME(first_day_2k25, 2025, 1, 1, 11, 0, 0, 3);  // Wednesday
}

// ------------------------------
// Special Cases Tests
// ------------------------------

TEST_F(TimeTConversionTest, LeapYears)
{
    // February 29, 2024 12:00:00 UTC is 1709208000
    time_t leap_timestamp = 1709208000;

    struct tm* leap_info_local = localtime(&leap_timestamp);
    ASSERT_NOT_NULL(leap_info_local);
    EXPECT_EQ(2024 - 1900, leap_info_local->tm_year);
    EXPECT_EQ(2 - 1, leap_info_local->tm_mon);
    EXPECT_EQ(29, leap_info_local->tm_mday);
    EXPECT_EQ(13, leap_info_local->tm_hour);
    EXPECT_EQ(0, leap_info_local->tm_min);
    EXPECT_EQ(0, leap_info_local->tm_sec);
    EXPECT_EQ(4, leap_info_local->tm_wday);  // Thursday
    EXPECT_EQ(59, leap_info_local->tm_yday);

    struct tm* leap_info_gmt = gmtime(&leap_timestamp);
    ASSERT_NOT_NULL(leap_info_gmt);
    EXPECT_EQ(2024 - 1900, leap_info_gmt->tm_year);
    EXPECT_EQ(2 - 1, leap_info_gmt->tm_mon);
    EXPECT_EQ(29, leap_info_gmt->tm_mday);
    EXPECT_EQ(12, leap_info_gmt->tm_hour);
    EXPECT_EQ(0, leap_info_gmt->tm_min);
    EXPECT_EQ(0, leap_info_gmt->tm_sec);
    EXPECT_EQ(4, leap_info_gmt->tm_wday);  // Thursday
    EXPECT_EQ(59, leap_info_gmt->tm_yday);

    const time_t non_leap_mar1 = CreateTimestamp(2023, 3, 1, 12, 0, 0);

    struct tm* non_leap_local = localtime(&non_leap_mar1);
    EXPECT_EQ(59, non_leap_local->tm_yday);

    struct tm* non_leap_gmt = gmtime(&non_leap_mar1);
    EXPECT_EQ(59, non_leap_gmt->tm_yday);

    const time_t leap_mar1 = CreateTimestamp(2024, 3, 1, 12, 0, 0);

    struct tm* leap_mar1_local = localtime(&leap_mar1);
    EXPECT_EQ(60, leap_mar1_local->tm_yday);

    struct tm* leap_mar1_gmt = gmtime(&leap_mar1);
    EXPECT_EQ(60, leap_mar1_gmt->tm_yday);
}

TEST_F(TimeTConversionTest, HistoricalDates)
{
    static constexpr time_t epoch_start = 0;
    VERIFY_LOCALTIME(epoch_start, 1970, 1, 1, 1, 0, 0, 4);  // Thursday
    VERIFY_GMTIME(epoch_start, 1970, 1, 1, 0, 0, 0, 4);     // Thursday

    static constexpr time_t historical_summer = 329918400;
    VERIFY_LOCALTIME(historical_summer, 1980, 6, 15, 13, 0, 0, 0);  // Sunday
    VERIFY_GMTIME(historical_summer, 1980, 6, 15, 12, 0, 0, 0);     // Sunday

    static constexpr time_t y2k_timestamp = 946684799;
    VERIFY_LOCALTIME(y2k_timestamp, 2000, 1, 1, 0, 59, 59, 6);  // Saturday
    VERIFY_GMTIME(y2k_timestamp, 1999, 12, 31, 23, 59, 59, 5);  // Friday
}

TEST_F(TimeTConversionTest, LocaltimeVsGmtime)
{
    // March 15, 2024 12:00:00 UTC is 1710504000
    time_t timestamp = 1710504000;

    struct tm* local = localtime(&timestamp);
    struct tm* gmt   = gmtime(&timestamp);

    ASSERT_NOT_NULL(local);
    ASSERT_NOT_NULL(gmt);

    EXPECT_EQ(13, local->tm_hour);
    EXPECT_EQ(12, gmt->tm_hour);

    EXPECT_EQ(gmt->tm_year, local->tm_year);
    EXPECT_EQ(gmt->tm_mon, local->tm_mon);
    EXPECT_EQ(gmt->tm_mday, local->tm_mday);

    // July 18, 2024 17:00:00 UTC is 1721322000
    timestamp = 1721322000;

    local = localtime(&timestamp);
    gmt   = gmtime(&timestamp);

    ASSERT_NOT_NULL(local);
    ASSERT_NOT_NULL(gmt);

    EXPECT_EQ(18, local->tm_hour);
    EXPECT_EQ(17, gmt->tm_hour);
}

TEST_F(TimeTConversionTest, DateComponents)
{
    time_t timestamp = CreateTimestamp(2024, 7, 15, 12, 30, 45);

    // Test localtime components
    struct tm* local_info = localtime(&timestamp);
    ASSERT_NOT_NULL(local_info);
    EXPECT_EQ(2024 - 1900, local_info->tm_year);
    EXPECT_EQ(7 - 1, local_info->tm_mon);
    EXPECT_EQ(15, local_info->tm_mday);
    EXPECT_EQ(13, local_info->tm_hour);
    EXPECT_EQ(30, local_info->tm_min);
    EXPECT_EQ(45, local_info->tm_sec);
    EXPECT_EQ(1, local_info->tm_wday);  // Monday
    EXPECT_EQ(196, local_info->tm_yday);

    // Test gmtime components
    struct tm* gmt_info = gmtime(&timestamp);
    ASSERT_NOT_NULL(gmt_info);
    EXPECT_EQ(2024 - 1900, gmt_info->tm_year);
    EXPECT_EQ(7 - 1, gmt_info->tm_mon);
    EXPECT_EQ(15, gmt_info->tm_mday);
    EXPECT_EQ(12, gmt_info->tm_hour);
    EXPECT_EQ(30, gmt_info->tm_min);
    EXPECT_EQ(45, gmt_info->tm_sec);
    EXPECT_EQ(1, gmt_info->tm_wday);  // Monday
    EXPECT_EQ(196, gmt_info->tm_yday);
}
