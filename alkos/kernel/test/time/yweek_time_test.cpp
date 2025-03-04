/* internal includes */
#include <assert.h>
#include <time.h>
#include <extensions/time.hpp>
#include <test_module/test.hpp>

class WeekCalculationTest : public TestGroupBase
{
    protected:
    static tm CreateTm(
        const int year, const int month, const int day, const int hour = 0, const int min = 0,
        const int sec = 0
    )
    {
        tm time{};
        time.tm_year  = year - kTmBaseYear;
        time.tm_mon   = month - 1;
        time.tm_mday  = day;
        time.tm_hour  = hour;
        time.tm_min   = min;
        time.tm_sec   = sec;
        time.tm_isdst = -1;
        return time;
    }

#define VerifyMondayBasedWeek(year, month, day, expectedWeek) \
    {                                                         \
        const tm date      = CreateTm(year, month, day);      \
        const int64_t week = CalculateMondayBasedWeek(date);  \
        EXPECT_EQ(expectedWeek, week);                        \
    }

#define VerifySundayBasedWeek(year, month, day, expectedWeek) \
    {                                                         \
        const tm date      = CreateTm(year, month, day);      \
        const int64_t week = CalculateSundayBasedWeek(date);  \
        EXPECT_EQ(expectedWeek, week);                        \
    }

#define VerifyIsoBasedWeek(year, month, day, expectedWeek) \
    {                                                      \
        const tm date      = CreateTm(year, month, day);   \
        const int64_t week = CalculateIsoBasedWeek(date);  \
        EXPECT_EQ(expectedWeek, week);                     \
    }
};

// ------------------------------
// Monday based
// ------------------------------

TEST_F(WeekCalculationTest, MondayBasedWeek)
{
    // First week of 2025 (Jan 1st is Wednesday, so it's in week 0)
    VerifyMondayBasedWeek(2025, 1, 1, 0);
    VerifyMondayBasedWeek(2025, 1, 2, 0);
    VerifyMondayBasedWeek(2025, 1, 3, 0);
    VerifyMondayBasedWeek(2025, 1, 4, 0);
    VerifyMondayBasedWeek(2025, 1, 5, 0);
    VerifyMondayBasedWeek(2025, 1, 6, 1);

    VerifyMondayBasedWeek(2025, 6, 15, 23);
    VerifyMondayBasedWeek(2025, 6, 16, 24);
    VerifyMondayBasedWeek(2024, 12, 30, 53);  // Monday of last week of 2024
    VerifyMondayBasedWeek(2024, 12, 31, 53);  // Tuesday of last week of 2024
    VerifyMondayBasedWeek(2024, 2, 29, 9);
    VerifyMondayBasedWeek(2025, 3, 3, 9);
    VerifyMondayBasedWeek(2025, 3, 9, 9);
    VerifyMondayBasedWeek(2025, 4, 30, 17);
    VerifyMondayBasedWeek(2025, 5, 1, 17);
}

// ------------------------------
// Sunday based
// ------------------------------

TEST_F(WeekCalculationTest, SundayBasedWeek)
{
    // First week of 2025 (Jan 1st is Wednesday)
    VerifySundayBasedWeek(2025, 1, 1, 0);
    VerifySundayBasedWeek(2025, 1, 4, 0);
    VerifySundayBasedWeek(2025, 1, 5, 1);

    VerifySundayBasedWeek(2025, 6, 14, 23);
    VerifySundayBasedWeek(2025, 6, 15, 24);   // Sunday starts new week
    VerifySundayBasedWeek(2024, 12, 29, 52);  // Sunday of last week of 2024
    VerifySundayBasedWeek(2024, 12, 31, 52);  // Tuesday of last week of 2024
    VerifySundayBasedWeek(2024, 2, 29, 8);
    VerifySundayBasedWeek(2025, 3, 2, 9);    // A Sunday (first day of week)
    VerifySundayBasedWeek(2025, 3, 8, 9);    // A Saturday (last day of week)
    VerifySundayBasedWeek(2025, 4, 30, 17);  // Wednesday
    VerifySundayBasedWeek(2025, 5, 1, 17);   // Thursday
}

// ------------------------------
// ISO based
// ------------------------------

TEST_F(WeekCalculationTest, IsoBasedWeek)
{
    // VerifyIsoBasedWeek(2024, 12, 29, 52);  // Sunday, last week of 2024
    VerifyIsoBasedWeek(2024, 12, 30, 1);  // Monday, first ISO week of 2025
    VerifyIsoBasedWeek(2024, 12, 31, 1);  // Tuesday, first ISO week of 2025

    VerifyIsoBasedWeek(2025, 1, 1, 1);  // Wednesday, first ISO week of 2025
    VerifyIsoBasedWeek(2025, 1, 5, 1);  // Sunday, still in first ISO week
    VerifyIsoBasedWeek(2025, 1, 6, 2);  // Monday, second ISO week starts
    VerifyIsoBasedWeek(2025, 1, 5, 1);  // Monday, second ISO week starts

    VerifyIsoBasedWeek(2024, 1, 1, 1);
    VerifyIsoBasedWeek(2024, 1, 4, 1);
    VerifyIsoBasedWeek(2024, 1, 5, 1);
    VerifyIsoBasedWeek(2024, 1, 6, 1);
    VerifyIsoBasedWeek(2024, 1, 7, 1);
    VerifyIsoBasedWeek(2024, 1, 14, 2);
    VerifyIsoBasedWeek(2024, 1, 8, 2);
    VerifyIsoBasedWeek(2024, 1, 21, 3);
    VerifyIsoBasedWeek(2024, 1, 15, 3);
    VerifyIsoBasedWeek(2024, 1, 28, 4);
    VerifyIsoBasedWeek(2024, 1, 29, 5);

    VerifyIsoBasedWeek(2025, 6, 15, 24);
    VerifyIsoBasedWeek(2025, 6, 16, 25);
    VerifyIsoBasedWeek(2025, 12, 28, 52);  // Sunday, last full ISO week of 2025
    VerifyIsoBasedWeek(2025, 12, 29, 1);   // Monday, first ISO week of 2026
    VerifyIsoBasedWeek(2025, 12, 31, 1);   // Wednesday, first ISO week of 2026
    VerifyIsoBasedWeek(2024, 2, 29, 9);
    VerifyIsoBasedWeek(2026, 1, 1, 1);    // Thursday, first week of year
    VerifyIsoBasedWeek(2027, 1, 1, 53);   // Friday, still last week of 2026
    VerifyIsoBasedWeek(2025, 3, 31, 14);  // Monday
    VerifyIsoBasedWeek(2025, 4, 1, 14);   // Tuesday
}
