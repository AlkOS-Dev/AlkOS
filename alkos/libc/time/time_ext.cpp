#include <assert.h>
#include <extensions/time.hpp>
#include <extensions/types.hpp>

// ------------------------------
// Implementations
// ------------------------------

u64 ConvertDateTimeToPosix(const tm &date_time, const timezone &time_zone)
{
    const i64 is_month_negative = date_time.tm_mon < 0;

    /* when month is negative, we goes backward */
    const i64 month_remainder = (date_time.tm_mon % 12) + is_month_negative * 12;
    const i64 month_years     = date_time.tm_mon / 12;
    const i64 years           = date_time.tm_year + month_years;

    /* we should not count the current month */
    const i64 days = kDaysInMonth[IsTmYearLeap(years)][month_remainder];

    i64 time = date_time.tm_sec;
    time += static_cast<i64>(date_time.tm_min) * kSecondsInMinute;
    time += static_cast<i64>(date_time.tm_hour) * kSecondsInHour;
    time += static_cast<i64>(date_time.tm_mday - 1) * kSecondsInDay;
    time += static_cast<i64>(days) * kSecondsInDay;
    time += static_cast<i64>(years) * kSecondsInUsualYear;

    /* adjust by leap years */
    time += years / 4 * kSecondsInDay;
    time -= years / 100 * kSecondsInDay;
    time += years / 400 * kSecondsInDay;

    /* adjust to fit in posix */
    time -= kPosixEpochTmSecondDiff;

    /* adjust by timezone */
    time -= static_cast<i64>(time_zone.west_offset_minutes * kSecondsInMinute);

    /* adjust by DST */
    if (date_time.tm_isdst > 0 && time_zone.dst_time_offset_minutes != 0) {
        time -= static_cast<i64>(time_zone.dst_time_offset_minutes * kSecondsInMinute);
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

i64 CalculateIsoBasedWeek(const tm &time)
{
    const i64 jan1_weekday            = GetWeekdayJan1(SumUpDays(time));
    const i64 normalized_jan1_weekday = jan1_weekday == 0 ? 7 : jan1_weekday;

    /* Check if current year starts with first week or week from previous year */
    const i64 days_in_first_week          = 8 - normalized_jan1_weekday;
    const bool is_first_week_in_this_year = days_in_first_week >= 4;

    /* Check for first days in year */
    const i64 days_sum = SumYearDays(time);

    if (days_sum <= days_in_first_week) {
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
    const i64 total_years_days           = IsTmYearLeap(time) ? 366 : 365;
    const i64 days_except_first_week     = total_years_days - days_in_first_week;
    const i64 days_in_last_not_full_week = days_except_first_week % 7;
    const i64 days_before_last_week      = total_years_days - days_in_last_not_full_week;
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

i64 CalculateIsoBasedYear(const tm &time)
{
    const i64 jan1_weekday            = GetWeekdayJan1(SumUpDays(time));
    const i64 normalized_jan1_weekday = jan1_weekday == 0 ? 7 : jan1_weekday;

    /* Check if current year starts with first week or week from previous year */
    const i64 days_in_first_week = 8 - normalized_jan1_weekday;

    /* Check for first days in year */
    const i64 days_sum = SumYearDays(time);
    if (days_sum <= days_in_first_week) {
        if (const bool is_first_week_in_this_year = days_in_first_week >= 4;
            !is_first_week_in_this_year) {
            return kTmBaseYear + time.tm_year - 1;
        }

        return kTmBaseYear + time.tm_year;
    }

    /* Check for last week of year for correction */
    const i64 total_years_days           = IsTmYearLeap(time) ? 366 : 365;
    const i64 days_except_first_week     = total_years_days - days_in_first_week;
    const i64 days_in_last_not_full_week = days_except_first_week % 7;
    const i64 days_before_last_week      = total_years_days - days_in_last_not_full_week;
    const bool is_last_not_full_week_from_next_year =
        days_in_last_not_full_week < 4 && days_in_last_not_full_week > 0;

    /* calculate last week separately */
    if (is_last_not_full_week_from_next_year && days_sum > days_before_last_week) {
        return kTmBaseYear + time.tm_year + 1;
    }

    return kTmBaseYear + time.tm_year;
}

std::tuple<u64, u64> CalculateMonthAndDaysFromPosix(const u64 days, const bool is_leap_year)
{
    ASSERT_LT(days, 366_u64);

    for (size_t idx = 1; idx < 12; ++idx) {
        if (days < kDaysInMonth[is_leap_year][idx]) {
            return {idx, days - kDaysInMonth[is_leap_year][idx - 1] + 1};
        }
    }

    return {12, days - kDaysInMonth[is_leap_year][11] + 1};
}

std::tuple<u64, u64> CalculateYears30MoreWLeaps(const u64 time)
{
    i64 local_time_left;
    i64 years = static_cast<i64>((time - kFirst30PosixYears) / kSecondsInUsualYear);
    [[maybe_unused]] i64 iterations = 0;

    do {
        ASSERT_LT(iterations++, 5);

        local_time_left = static_cast<i64>(time - kFirst30PosixYears);

        local_time_left -= years * kSecondsInUsualYear;

        /* Adjust by leap years div 4 */
        local_time_left -= years / 4 * kSecondsInDay;

        /* Adjust by leap years div 100 */
        local_time_left += years / 100 * kSecondsInDay;

        /* Adjust by leap years div 400 */
        local_time_left -= years / 400 * kSecondsInDay;

        /* prepare for next iteration */
        years -= 1;
    } while (local_time_left < 0);

    ASSERT_GE(local_time_left, 0_i64);
    return {years + 1 + 30, static_cast<u64>(local_time_left)};
}

std::tuple<u64, u64> CalculateYears30LessWLeaps(const u64 time)
{
    i64 local_time_left;
    i64 years                       = time / kSecondsInUsualYear;
    [[maybe_unused]] i64 iterations = 0;

    do {
        ASSERT_LT(iterations++, 2);

        local_time_left = static_cast<i64>(time);

        local_time_left -= years * kSecondsInUsualYear;

        /* Adjust by leap years */
        local_time_left -= years > 2 ? (years - 2) / 4 * kSecondsInDay : 0;

        /* prepare for next iteration */
        years -= 1;
    } while (local_time_left < 0);

    return {years + 1, static_cast<u64>(local_time_left)};
}

u64 GetDSTOffset(u64 time, const timezone &time_zone)
{
    if (time_zone.dst_time_offset_minutes == 0) {
        return 0;
    }

    if (time_zone.dst_time_start_seconds < time_zone.dst_time_end_seconds) {
        /* Continuous DST */

        if (time >= time_zone.dst_time_start_seconds && time < time_zone.dst_time_end_seconds) {
            return time_zone.dst_time_offset_minutes * kSecondsInMinute;
        } else {
            return 0;
        }
    } else {
        /* Spans over the year */

        if (time >= time_zone.dst_time_start_seconds || time < time_zone.dst_time_end_seconds) {
            return time_zone.dst_time_offset_minutes * kSecondsInMinute;
        } else {
            return 0;
        }
    }
}

tm *ConvertFromPosixToTm(const time_t timer, tm &result, const timezone &tz)
{
    u64 time_left = timer;

    /* add local time offset */
    time_left += tz.west_offset_minutes * kSecondsInMinute;

    const auto [years, time_left_after_years] = CalculateYearsFromPosix(time_left);

    /* Apply years */
    time_left      = time_left_after_years;
    result.tm_year = static_cast<int>(kPosixToTmYearDiff + years);

    /* Check if DST is in effect */
    const u64 dst_offset = GetDSTOffset(time_left, tz);
    time_left += dst_offset;
    result.tm_isdst = static_cast<int>(dst_offset != 0_u64 ? 1_u64 : dst_offset);

    /* Apply days */
    const u64 days = time_left / kSecondsInDay;
    time_left -= days * kSecondsInDay;
    result.tm_yday = static_cast<int>(days);

    /* Apply hours */
    const u64 hours = time_left / kSecondsInHour;
    time_left -= hours * kSecondsInHour;
    result.tm_hour = static_cast<int>(hours);

    /* Apply minutes */
    const u64 minutes = time_left / kSecondsInMinute;
    time_left -= minutes * kSecondsInMinute;
    result.tm_min = static_cast<int>(minutes);

    /* Apply seconds */
    result.tm_sec = static_cast<int>(time_left);

    /* Apply month and days */
    const auto [month, day] = CalculateMonthAndDaysFromPosix(days, IsTmYearLeap(result));
    result.tm_mon           = static_cast<int>(month - 1);
    result.tm_mday          = static_cast<int>(day);
    result.tm_wday          = static_cast<int>(CalculateDayOfWeek(result));

    return &result;
}
