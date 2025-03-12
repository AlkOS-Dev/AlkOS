/* internal includes */
#include <assert.h>
#include <memory.h>
#include <time.h>
#include <extensions/time.hpp>
#include <test_module/test.hpp>

class LocaltimeTest : public TestGroupBase
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

        return mktime(&timeinfo);
    }
};

// Macros for testing localtime (without while loop)
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

// ------------------------------
// Basic Localtime Tests
// ------------------------------

TEST_F(LocaltimeTest, BasicLocaltimeConversion)
{
    time_t timestamp = CreateTimestamp(2024, 1, 15, 12, 30, 45);
    VERIFY_LOCALTIME(timestamp, 2024, 1, 15, 12, 30, 45, 1);  // Monday

    timestamp = CreateTimestamp(2024, 7, 15, 12, 30, 45);
    VERIFY_LOCALTIME(timestamp, 2024, 7, 15, 12, 30, 45, 1);  // Monday
}

// ------------------------------
// Manual POSIX Timestamp Tests
// ------------------------------

TEST_F(LocaltimeTest, ManualTimestamps)
{
    // Test timestamps defined directly without using mktime

    // 2024-02-15 12:30:45 UTC is 1707999045
    // In Warsaw (UTC+1 in February), this should be 13:30:45
    time_t feb15_timestamp = 1707999045;
    VERIFY_LOCALTIME(feb15_timestamp, 2024, 2, 15, 13, 30, 45, 4);  // Thursday

    // 2024-06-15 12:30:45 UTC is 1718457045
    // In Warsaw (UTC+2 in June), this should be 14:30:45
    time_t jun15_timestamp = 1718457045;
    VERIFY_LOCALTIME(jun15_timestamp, 2024, 6, 15, 14, 30, 45, 6);  // Saturday

    // 2024-03-31 00:59:59 UTC is 1711839599
    // In Warsaw, this is just before DST starts, should be 01:59:59 CET
    time_t dst_start_utc = 1711839599;
    VERIFY_LOCALTIME(dst_start_utc, 2024, 3, 31, 1, 59, 59, 0);  // Sunday

    // 2024-03-31 01:00:00 UTC is 1711839600
    // In Warsaw, this is right after DST starts, should be 03:00:00 CEST
    time_t after_dst_utc = 1711839600;
    VERIFY_LOCALTIME(after_dst_utc, 2024, 3, 31, 3, 0, 0, 0);  // Sunday

    // 2024-10-27 00:59:59 UTC is 1730127599
    // In Warsaw (still in CEST), this should be 02:59:59
    time_t before_dst_end_utc = 1730127599;
    VERIFY_LOCALTIME(before_dst_end_utc, 2024, 10, 27, 2, 59, 59, 0);  // Sunday

    // 2024-10-27 01:30:00 UTC is 1730129400
    // After DST ends, this should be 02:30:00 CET
    time_t after_dst_end_utc = 1730129400;
    VERIFY_LOCALTIME(after_dst_end_utc, 2024, 10, 27, 2, 30, 0, 0);  // Sunday
}

// ------------------------------
// Special Cases Tests
// ------------------------------

TEST_F(LocaltimeTest, LeapYears)
{
    // February 29, 2024 12:00:00 UTC is 1709208000
    // In Warsaw (UTC+1), this should be 13:00:00
    time_t leap_timestamp = 1709208000;
    struct tm* leap_info  = localtime(&leap_timestamp);
    ASSERT_NOT_NULL(leap_info);

    EXPECT_EQ(2024 - 1900, leap_info->tm_year);
    EXPECT_EQ(2 - 1, leap_info->tm_mon);
    EXPECT_EQ(29, leap_info->tm_mday);
    EXPECT_EQ(13, leap_info->tm_hour);
    EXPECT_EQ(0, leap_info->tm_min);
    EXPECT_EQ(0, leap_info->tm_sec);
    EXPECT_EQ(4, leap_info->tm_wday);   // Thursday
    EXPECT_EQ(60, leap_info->tm_yday);  // Day 60 in leap year

    // Day after February in non-leap year vs leap year
    time_t non_leap_mar1     = CreateTimestamp(2023, 3, 1, 12, 0, 0);
    struct tm* non_leap_info = localtime(&non_leap_mar1);
    EXPECT_EQ(60, non_leap_info->tm_yday);  // Day 60 in non-leap year

    time_t leap_mar1          = CreateTimestamp(2024, 3, 1, 12, 0, 0);
    struct tm* leap_mar1_info = localtime(&leap_mar1);
    EXPECT_EQ(61, leap_mar1_info->tm_yday);  // Day 61 in leap year
}

TEST_F(LocaltimeTest, HistoricalDates)
{
    static constexpr time_t epoch_start = 0;
    VERIFY_LOCALTIME(epoch_start, 1970, 1, 1, 1, 0, 0, 4);  // Thursday

    static constexpr time_t historical_summer = 329918400;
    VERIFY_LOCALTIME(historical_summer, 1980, 6, 15, 13, 0, 0, 0);  // Sunday

    static constexpr time_t y2k_timestamp = 946684799;
    VERIFY_LOCALTIME(y2k_timestamp, 2000, 1, 1, 0, 59, 59, 6);  // Saturday
}

TEST_F(LocaltimeTest, tmp) {}

TEST_F(LocaltimeTest, LocaltimeVsGmtime)
{
    // Compare localtime and gmtime for the same timestamp

    // March 15, 2024 12:00:00 UTC is 1710504000
    time_t timestamp = 1710504000;

    struct tm* local = localtime(&timestamp);
    struct tm* gmt   = gmtime(&timestamp);

    ASSERT_NOT_NULL(local);
    ASSERT_NOT_NULL(gmt);

    // Should be 13:00 in Warsaw (UTC+1)
    EXPECT_EQ(13, local->tm_hour);
    EXPECT_EQ(12, gmt->tm_hour);

    // Both should have the same date
    EXPECT_EQ(gmt->tm_year, local->tm_year);
    EXPECT_EQ(gmt->tm_mon, local->tm_mon);
    EXPECT_EQ(gmt->tm_mday, local->tm_mday);

    // July 15, 2024 12:00:00 UTC is 1721322000
    timestamp = 1721322000;

    local = localtime(&timestamp);
    gmt   = gmtime(&timestamp);

    ASSERT_NOT_NULL(local);
    ASSERT_NOT_NULL(gmt);

    // Should be 14:00 in Warsaw (UTC+2 in summer)
    EXPECT_EQ(14, local->tm_hour);
    EXPECT_EQ(12, gmt->tm_hour);
}

TEST_F(LocaltimeTest, DateComponents)
{
    time_t timestamp    = CreateTimestamp(2024, 7, 15, 12, 30, 45);
    struct tm* timeinfo = localtime(&timestamp);
    ASSERT_NOT_NULL(timeinfo);

    // Test all individual components
    EXPECT_EQ(2024 - 1900, timeinfo->tm_year);
    EXPECT_EQ(7 - 1, timeinfo->tm_mon);
    EXPECT_EQ(15, timeinfo->tm_mday);
    EXPECT_EQ(12, timeinfo->tm_hour);
    EXPECT_EQ(30, timeinfo->tm_min);
    EXPECT_EQ(45, timeinfo->tm_sec);
    EXPECT_EQ(1, timeinfo->tm_wday);    // Monday
    EXPECT_EQ(197, timeinfo->tm_yday);  // Day 197 of the year
}
