#include <assert.h>
#include <extensions/time.hpp>

// ------------------------------
// Constants
// ------------------------------

static constexpr uint16_t kDaysInMonth[2][13]{
    /* Normal Year */
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    /* Leap Year */
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};

// ------------------------------
// static functions
// ------------------------------

FAST_CALL int64_t SumUpDays_(const int64_t year)
{
    const int64_t years_adjusted = year - 1;

    return years_adjusted * 365 + years_adjusted / 4 - years_adjusted / 100 + years_adjusted / 400;
}

WRAP_CALL int64_t SumUpDays_(const tm &time_ptr)
{
    return SumUpDays_(time_ptr.tm_year + kTmBaseYear);
}

FAST_CALL int64_t GetWeekdayJan1_(const int64_t days_since_century)
{
    return (days_since_century + 1) % 7;
}

/**
 * @note: Includes the current day
 */
FAST_CALL int64_t SumYearDays_(const tm &time_ptr)
{
    const bool is_leap = IsTmYearLeap(time_ptr.tm_year);
    return kDaysInMonth[is_leap][time_ptr.tm_mon] + time_ptr.tm_mday;
}

FAST_CALL int64_t CalculateDayOfWeek(const tm &time)
{
    const int64_t total_days = SumUpDays_(time) + SumYearDays_(time) - 1;
    return total_days % 7;
}

int64_t CalculateISOWeek(const tm &time)
{
    // Convert Sunday from 0 to 7 for ISO calculations
    int64_t day_of_week = CalculateDayOfWeek(time);
    if (day_of_week == 0) {
        day_of_week = 7;
    }

    const int64_t day_of_year = SumYearDays_(time);

    // Convert Sunday from 0 to 7 for ISO calculations
    int64_t jan1_weekday = GetWeekdayJan1_(SumUpDays_(time));
    if (jan1_weekday == 0) {
        jan1_weekday = 7;
    }

    int64_t week_num = (day_of_year + 7 - day_of_week + (jan1_weekday - 1)) / 7;

    if (jan1_weekday > 4) {
        // The first week does not contain a Thursday, so the first Thursday is in week 2
        // Days before the first Thursday belong to the last week of the previous year
        if (day_of_year < (8 - jan1_weekday)) {
            // This date belongs to the last week of the previous year
            tm prev_year_last_day = time;
            prev_year_last_day.tm_year -= 1;
            prev_year_last_day.tm_mon  = 11;
            prev_year_last_day.tm_mday = 31;

            return CalculateISOWeek(prev_year_last_day);
        }
    } else {
        // The first week contains a Thursday, so it's week 1
        week_num = 1 + (day_of_year - 1 + (jan1_weekday - 1)) / 7;
    }

    // Check if we're in the first week of the next year
    if (week_num > 52) {
        // Get the weekday of January 1st of the next year
        tm next_year_first_day;
        next_year_first_day.tm_year = time.tm_year + 1;
        next_year_first_day.tm_mon  = 0;
        next_year_first_day.tm_mday = 1;

        int64_t next_jan1_weekday = GetWeekdayJan1_(SumUpDays_(next_year_first_day));
        if (next_jan1_weekday == 0) {
            next_jan1_weekday = 7;
        }

        // If January 1st of next year is on Monday to Thursday, then the current date
        // belongs to week 1 of next year
        if (next_jan1_weekday <= 4) {
            if (day_of_year >= (365 + IsTmYearLeap(time.tm_year) - 7 + day_of_week)) {
                return 1;
            }
        }
    }

    // Handle the case for the last week of the previous year
    if (week_num == 0) {
        // Get the weekday of December 31st of the previous year
        tm prev_year_last_day;
        prev_year_last_day.tm_year = time.tm_year - 1;
        prev_year_last_day.tm_mon  = 11;
        prev_year_last_day.tm_mday = 31;

        return CalculateISOWeek(prev_year_last_day);
    }

    return week_num;
}

// ------------------------------
// Implementations
// ------------------------------

uint64_t ConvertDateTimeToSeconds(const tm &date_time, const timezone &time_zone)
{
    const int64_t is_month_negative = date_time.tm_mon < 0;

    /* when month is negative, we goes backward */
    const int64_t month_remainder = (date_time.tm_mon % 12) + is_month_negative * 12;
    const int64_t month_years     = date_time.tm_mon / 12;
    const int64_t years           = date_time.tm_year + month_years;

    /* we should not count the current month */
    const int64_t days = kDaysInMonth[IsTmYearLeap(years)][month_remainder];

    int64_t time = date_time.tm_sec;
    time += static_cast<int64_t>(date_time.tm_min) * kSecondsInMinute;
    time += static_cast<int64_t>(date_time.tm_hour) * kSecondsInHour;
    time += static_cast<int64_t>(date_time.tm_mday - 1) * kSecondsInDay;
    time += static_cast<int64_t>(days) * kSecondsInDay;
    time += static_cast<int64_t>(years) * kSecondsInUsualYear;

    /* adjust by leap years */
    time += years / 4 * kSecondsInDay;
    time -= years / 100 * kSecondsInDay;
    time += years / 400 * kSecondsInDay;

    /* adjust to fit in posix */
    time -= kPosixEpochTmSecondDiff;

    /* adjust by timezone */
    time -= static_cast<int64_t>(time_zone.west_offset_minutes * kSecondsInMinute);

    /* adjust by DST */
    if (date_time.tm_isdst > 0 && time_zone.dst_time_offset_minutes != 0) {
        time -= static_cast<int64_t>(time_zone.dst_time_offset_minutes * kSecondsInMinute);
    }

    /* determine if DST is in effect */
    if (date_time.tm_isdst < 0) {
        /* TODO: here we should try to determine if DST is in effect */
        TODO_BY_THE_END_OF_MILESTONE1
        assert(false && "DST guess is not supported yet");
    }

    if (time < 0) {
        return kConversionFailed;
    }

    return time;
}

int64_t CalculateMondayBasedWeek(const tm &time) { return CalculateISOWeek(time); }

int64_t CalculateSundayBasedWeek(const tm &time)
{
    const int64_t jan1_weekday = GetWeekdayJan1_(SumUpDays_(time));
    const int64_t days         = SumYearDays_(time) - 1;

    return (days + jan1_weekday) / 7;
}
