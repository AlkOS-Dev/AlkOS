#include <assert.h>
#include <extensions/time.hpp>

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

bool ValidateTm(const tm &time_ptr)
{
    return time_ptr.tm_hour >= 0 && time_ptr.tm_hour <= 23 && time_ptr.tm_min >= 0 &&
           time_ptr.tm_min <= 59 && time_ptr.tm_sec >= 0 && time_ptr.tm_sec <= 60 &&
           time_ptr.tm_mon >= 0 && time_ptr.tm_mon <= 11 && time_ptr.tm_mday >= 1 &&
           time_ptr.tm_mday <= 31 && time_ptr.tm_year >= 0 && time_ptr.tm_year <= 9999 &&
           time_ptr.tm_wday >= 0 && time_ptr.tm_wday <= 6 && time_ptr.tm_yday >= 0 &&
           time_ptr.tm_yday <= 365;
}

int64_t CalculateIsoBasedWeek(const tm &time)
{
    const int64_t jan1_weekday            = GetWeekdayJan1(SumUpDays(time));
    const int64_t normalized_jan1_weekday = jan1_weekday == 0 ? 7 : jan1_weekday;

    /* Check if current year starts with first week or week from previous year */
    const int64_t days_in_first_week      = 8 - normalized_jan1_weekday;
    const bool is_first_week_in_this_year = days_in_first_week >= 4;

    /* Check for first days in year */
    const int64_t days_sum = SumYearDays(time);

    if (const bool is_in_first_days = days_sum <= days_in_first_week) {
        if (!is_first_week_in_this_year) {
            /* Extract week number from previous year if first week is not 4 days long */
            tm time_prev{};
            time_prev.tm_year = time.tm_year - 1;
            time_prev.tm_mon  = 11;
            time_prev.tm_mday = 31;

            return CalculateIsoBasedWeek(time_prev);
        }

        return 1;
    }

    /* Check for last week of year for correction */
    const int64_t total_years_days           = IsTmYearLeap(time) ? 366 : 365;
    const int64_t days_except_first_week     = total_years_days - days_in_first_week;
    const int64_t days_in_last_not_full_week = days_except_first_week % 7;
    const int64_t days_before_last_week      = total_years_days - days_in_last_not_full_week;
    const bool is_last_not_full_week_from_next_year =
        days_in_last_not_full_week < 4 && days_in_last_not_full_week > 0;

    /* calculate last week separately */
    if (is_last_not_full_week_from_next_year && days_sum > days_before_last_week) {
        /* Extract week number from next year if last week is shorter than 4 days */
        tm time_next{};
        time_next.tm_year = time.tm_year + 1;
        time_next.tm_mon  = 0;
        time_next.tm_mday = 1;

        return CalculateIsoBasedWeek(time_next);
    }

    return 1 + is_first_week_in_this_year + (days_sum - days_in_first_week - 1) / 7;
}

int64_t CalculateIsoBasedYear(const tm &time)
{
    const int64_t jan1_weekday            = GetWeekdayJan1(SumUpDays(time));
    const int64_t normalized_jan1_weekday = jan1_weekday == 0 ? 7 : jan1_weekday;

    /* Check if current year starts with first week or week from previous year */
    const int64_t days_in_first_week = 8 - normalized_jan1_weekday;

    /* Check for first days in year */
    const int64_t days_sum = SumYearDays(time);
    if (days_sum <= days_in_first_week) {
        if (const bool is_first_week_in_this_year = days_in_first_week >= 4;
            !is_first_week_in_this_year) {
            return kTmBaseYear + time.tm_year - 1;
        }

        return kTmBaseYear + time.tm_year;
    }

    /* Check for last week of year for correction */
    const int64_t total_years_days           = IsTmYearLeap(time) ? 366 : 365;
    const int64_t days_except_first_week     = total_years_days - days_in_first_week;
    const int64_t days_in_last_not_full_week = days_except_first_week % 7;
    const int64_t days_before_last_week      = total_years_days - days_in_last_not_full_week;
    const bool is_last_not_full_week_from_next_year =
        days_in_last_not_full_week < 4 && days_in_last_not_full_week > 0;

    /* calculate last week separately */
    if (is_last_not_full_week_from_next_year && days_sum > days_before_last_week) {
        return kTmBaseYear + time.tm_year + 1;
    }

    return kTmBaseYear + time.tm_year;
}
