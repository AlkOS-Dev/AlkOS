/* internal includes */
#include <assert.h>
#include <memory.h>
#include <time.h>
#include <extensions/time.hpp>
#include <test_module/test.hpp>

class StrftimeTestFixture : public TestGroupBase
{
    protected:
    static const size_t kBufferSize = 128;
    char buffer_[kBufferSize];

    static tm CreateTm(
        const int year, const int month, const int day, const int wday, const int hour = 0,
        const int min = 0, const int sec = 0
    )
    {
        tm time{};
        time.tm_year  = year - 1900;
        time.tm_mon   = month - 1;
        time.tm_mday  = day;
        time.tm_hour  = hour;
        time.tm_min   = min;
        time.tm_sec   = sec;
        time.tm_isdst = -1;
        time.tm_wday  = wday;

        return time;
    }

#define VerifyStrftime(time, format, expected)                               \
    {                                                                        \
        memset(buffer_, 0, kBufferSize);                                     \
        const size_t result = strftime(buffer_, kBufferSize, format, &time); \
        EXPECT_GT(result, 0);                                                \
        EXPECT_STREQ(expected, buffer_);                                     \
    }
};

// ------------------------------
// Basic Format Tests
// ------------------------------

TEST_F(StrftimeTestFixture, BasicDateFormats)
{
    tm date = CreateTm(2024, 3, 15, 5, 14, 30, 45);  // Friday

    VerifyStrftime(date, "%Y-%m-%d", "2024-03-15");
    VerifyStrftime(date, "%d/%m/%Y", "15/03/2024");
    VerifyStrftime(date, "%m/%d/%y", "03/15/24");
    VerifyStrftime(date, "%B %d, %Y", "March 15, 2024");
    VerifyStrftime(date, "%A, %B %d, %Y", "Friday, March 15, 2024");
}

TEST_F(StrftimeTestFixture, BasicTimeFormats)
{
    tm date = CreateTm(2024, 3, 15, 5, 14, 30, 45);  // Friday

    VerifyStrftime(date, "%H:%M:%S", "14:30:45");
    VerifyStrftime(date, "%I:%M:%S %p", "02:30:45 PM");
    VerifyStrftime(date, "%r", "02:30:45 PM");
    VerifyStrftime(date, "%R", "14:30");
    VerifyStrftime(date, "%T", "14:30:45");
}

TEST_F(StrftimeTestFixture, CombinedDateTimeFormats)
{
    tm date = CreateTm(2024, 3, 15, 5, 14, 30, 45);  // Friday

    VerifyStrftime(date, "%Y-%m-%d %H:%M:%S", "2024-03-15 14:30:45");
    VerifyStrftime(date, "%a %b %d %T %Y", "Fri Mar 15 14:30:45 2024");
    VerifyStrftime(date, "%F %T", "2024-03-15 14:30:45");
}

// ------------------------------
// Week Number Tests
// ------------------------------

TEST_F(StrftimeTestFixture, WeekNumbers)
{
    // ISO week numbers (%V) - Week containing Jan 4
    VerifyStrftime(CreateTm(2024, 1, 1, 1), "%G-W%V-%u", "2024-W01-1");  // Monday of week 1
    VerifyStrftime(
        CreateTm(2023, 1, 1, 0), "%G-W%V-%u", "2022-W52-7"
    );  // Sunday of week 52 (prev year)
    VerifyStrftime(
        CreateTm(2024, 12, 31, 2), "%G-W%V-%u", "2025-W01-2"
    );  // Tuesday of week 1 (next year)

    // Sunday-based week numbers (%U) - First Sunday starts week 1
    VerifyStrftime(
        CreateTm(2024, 1, 1, 1), "%Y-WU%U-%w", "2024-WU00-1"
    );  // Monday before first Sunday
    VerifyStrftime(CreateTm(2024, 1, 7, 0), "%Y-WU%U-%w", "2024-WU01-0");  // First Sunday = week 1

    // Monday-based week numbers (%W) - First Monday starts week 1
    VerifyStrftime(CreateTm(2024, 1, 1, 1), "%Y-WW%W-%w", "2024-WW01-1");  // First Monday = week 1
    VerifyStrftime(
        CreateTm(2023, 1, 1, 0), "%Y-WW%W-%w", "2023-WW00-0"
    );  // Sunday before first Monday
}

// ------------------------------
// Special Cases Tests
// ------------------------------

TEST_F(StrftimeTestFixture, LeapYears)
{
    // February 29 in leap years
    VerifyStrftime(
        CreateTm(2020, 2, 29, 6), "%Y-%m-%d is day %j of the year",
        "2020-02-29 is day 060 of the year"
    );
    VerifyStrftime(
        CreateTm(2024, 2, 29, 4), "%Y-%m-%d is day %j of the year",
        "2024-02-29 is day 060 of the year"
    );

    // February 28 in non-leap year vs leap year
    VerifyStrftime(
        CreateTm(2023, 2, 28, 2), "%Y-%m-%d is day %j of the year",
        "2023-02-28 is day 059 of the year"
    );
    VerifyStrftime(
        CreateTm(2024, 2, 28, 3), "%Y-%m-%d is day %j of the year",
        "2024-02-28 is day 059 of the year"
    );

    // March 1 in non-leap year vs leap year
    VerifyStrftime(
        CreateTm(2023, 3, 1, 3), "%Y-%m-%d is day %j of the year",
        "2023-03-01 is day 060 of the year"
    );
    VerifyStrftime(
        CreateTm(2024, 3, 1, 5), "%Y-%m-%d is day %j of the year",
        "2024-03-01 is day 061 of the year"
    );
}

TEST_F(StrftimeTestFixture, FormatSpecifiers)
{
    tm date = CreateTm(2024, 3, 15, 5, 14, 30, 45);  // Friday

    VerifyStrftime(date, "Century: %C", "Century: 20");
    VerifyStrftime(date, "Year without century: %y", "Year without century: 24");
    VerifyStrftime(date, "Year with century: %Y", "Year with century: 2024");
    VerifyStrftime(date, "Month as number: %m", "Month as number: 03");
    VerifyStrftime(date, "Month name: %B", "Month name: March");
    VerifyStrftime(date, "Month abbreviation: %b", "Month abbreviation: Mar");
    VerifyStrftime(date, "Day of month: %d", "Day of month: 15");
    VerifyStrftime(date, "Day of year: %j", "Day of year: 075");
    VerifyStrftime(date, "Hour (24h): %H", "Hour (24h): 14");
    VerifyStrftime(date, "Hour (12h): %I", "Hour (12h): 02");
    VerifyStrftime(date, "AM/PM: %p", "AM/PM: PM");
    VerifyStrftime(date, "Minute: %M", "Minute: 30");
    VerifyStrftime(date, "Second: %S", "Second: 45");
    VerifyStrftime(date, "Weekday name: %A", "Weekday name: Friday");
    VerifyStrftime(date, "Weekday abbreviation: %a", "Weekday abbreviation: Fri");
    VerifyStrftime(
        date, "Weekday as number (0-6, Sunday=0): %w", "Weekday as number (0-6, Sunday=0): 5"
    );
    VerifyStrftime(date, "Literal %% symbol: %%", "Literal % symbol: %");
}
