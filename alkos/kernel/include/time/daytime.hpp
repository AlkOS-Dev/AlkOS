#ifndef ALKOS_INCLUDE_TIME_DAYTIME_HPP_
#define ALKOS_INCLUDE_TIME_DAYTIME_HPP_

#include <sys/time.h>
#include <time.h>
#include <extensions/time.hpp>
#include <modules/timing_constants.hpp>

TIMING_DECL_START
class DayTime
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    DayTime();

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F time_t GetTime() const noexcept { return time_; }

    void SyncWithHardware();

    NODISCARD timezone GetTimezone() const { return time_zone_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    time_t time_{};

    TODO_TIMEZONES
    /* Hard coded Poland */
    static constexpr uint64_t kPolandOffset = 1;

    timezone time_zone_{
        .west_offset_minutes     = kPolandOffset * kMinutesInHour,
        .dst_time_offset_minutes = 0,
        .dst_time_start_seconds  = -1,
        .dst_time_end_seconds    = -1,
    };
};
TIMING_DECL_END

#endif  // ALKOS_INCLUDE_TIME_DAYTIME_HPP_
