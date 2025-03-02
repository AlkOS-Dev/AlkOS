/* internal includes */
#include <assert.h>
#include <time.h>
#include <extensions/time.hpp>
#include <test_module/test.hpp>

class WeekCalculationTestFixture : public TestGroupBase
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
};

// ------------------------------
// Monday based (ISO)
// ------------------------------

TEST_F(WeekCalculationTestFixture, MondayBasedWeek_StartOfYear)
{
    // Test January 1st for various years with different weekdays
    VerifyMondayBasedWeek(2020, 1, 1, 1);   // Wednesday of week 1
    VerifyMondayBasedWeek(2021, 1, 1, 53);  // Friday of week 53 (prev year)
    VerifyMondayBasedWeek(2022, 1, 1, 52);  // Saturday of week 52 (prev year)
    VerifyMondayBasedWeek(2023, 1, 1, 52);  // Sunday of week 52 (prev year)
    VerifyMondayBasedWeek(2024, 1, 1, 1);   // Monday of week 1
    VerifyMondayBasedWeek(2025, 1, 1, 1);   // Wednesday of week 1

    VerifyMondayBasedWeek(2023, 1, 2, 1);   // Monday of week 1
    VerifyMondayBasedWeek(2023, 1, 8, 1);   // Sunday of week 1
    VerifyMondayBasedWeek(2023, 1, 9, 2);   // Monday of week 2
    VerifyMondayBasedWeek(2023, 3, 6, 10);  // Monday of week 10
    VerifyMondayBasedWeek(2023, 3, 1, 9);   // Wednesday of week 9
    VerifyMondayBasedWeek(2023, 2, 28, 9);  // Tuesday of week 9
    VerifyMondayBasedWeek(2023, 2, 1, 5);   // Wednesday of week 5
    VerifyMondayBasedWeek(2023, 1, 16, 3);  // Monday of week 3
    VerifyMondayBasedWeek(2023, 1, 23, 4);  // Monday of week 4
}

TEST_F(WeekCalculationTestFixture, MondayBasedWeek_MidYear)
{
    VerifyMondayBasedWeek(2024, 6, 15, 24);  // Saturday of week 24
    VerifyMondayBasedWeek(2024, 7, 1, 27);   // Monday of week 27
    VerifyMondayBasedWeek(2025, 5, 12, 20);  // Monday of week 20
    VerifyMondayBasedWeek(2023, 8, 20, 33);  // Sunday of week 33
}

TEST_F(WeekCalculationTestFixture, MondayBasedWeek_EndOfYear)
{
    VerifyMondayBasedWeek(2024, 12, 31, 1);   // Tuesday of week 1 (next year)
    VerifyMondayBasedWeek(2023, 12, 31, 52);  // Sunday of week 52
    VerifyMondayBasedWeek(2025, 12, 31, 1);   // Wednesday of week 1 (next year)
    VerifyMondayBasedWeek(2026, 12, 31, 53);  // Thursday of week 53
}

TEST_F(WeekCalculationTestFixture, MondayBasedWeek_LeapYears)
{
    VerifyMondayBasedWeek(2020, 2, 29, 9);  // Saturday of week 9
    VerifyMondayBasedWeek(2024, 2, 29, 9);  // Thursday of week 9
    VerifyMondayBasedWeek(2024, 3, 1, 9);   // Friday of week 9
    VerifyMondayBasedWeek(2028, 2, 29, 9);  // Tuesday of week 9
}

TEST_F(WeekCalculationTestFixture, MondayBasedWeek_EdgeCases)
{
    VerifyMondayBasedWeek(2024, 1, 1, 1);   // Monday of week 1
    VerifyMondayBasedWeek(2024, 1, 8, 2);   // Monday of week 2
    VerifyMondayBasedWeek(2024, 1, 15, 3);  // Monday of week 3

    VerifyMondayBasedWeek(2024, 1, 7, 1);   // Sunday of week 1
    VerifyMondayBasedWeek(2024, 1, 14, 2);  // Sunday of week 2
    VerifyMondayBasedWeek(2024, 1, 21, 3);  // Sunday of week 3
}

// ------------------------------
// Sunday based
// ------------------------------

TEST_F(WeekCalculationTestFixture, SundayBasedWeek_StartOfYear)
{
    // Test January 1st for various years
    VerifySundayBasedWeek(2020, 1, 1, 1);  // Wednesday of week 1
    VerifySundayBasedWeek(2021, 1, 1, 1);  // Friday of week 1
    VerifySundayBasedWeek(2022, 1, 1, 1);  // Saturday of week 1
    VerifySundayBasedWeek(2023, 1, 1, 1);  // Sunday of week 1
    VerifySundayBasedWeek(2024, 1, 1, 1);  // Monday of week 1
    VerifySundayBasedWeek(2025, 1, 1, 1);  // Wednesday of week 1
}
