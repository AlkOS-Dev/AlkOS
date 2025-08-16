#ifndef ALKOS_KERNEL_INCLUDE_TIME_SYSTEM_TIME_HPP_
#define ALKOS_KERNEL_INCLUDE_TIME_SYSTEM_TIME_HPP_

#include <sys/time.h>
#include <time.h>
#include <extensions/time.hpp>
#include "modules/timing_constants.hpp"

namespace timing
{
class SystemTime
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    SystemTime();

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F time_t GetTime() const noexcept
    {
        return boot_time_read_local_ + (GetSysLiveTimeNs() + kNanosInSecond / 2) / kNanosInSecond;
    }

    NODISCARD static time_t GetSysLiveTimeNs();

    void SyncWithHardware();

    NODISCARD timezone GetTimezone() const { return time_zone_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    time_t boot_time_read_utc_{};
    time_t boot_time_read_local_{};

    TODO_TIMEZONES
    /* Hard coded Poland */
    static constexpr uint64_t kPolandOffset = 1;

    timezone time_zone_{
        .west_offset_minutes     = kPolandOffset * kMinutesInHour,
        .dst_time_offset_minutes = 0,
        .dst_time_start_seconds  = static_cast<u16>(-1),
        .dst_time_end_seconds    = static_cast<u16>(-1),
    };
};
}  // namespace timing

#endif  // ALKOS_KERNEL_INCLUDE_TIME_SYSTEM_TIME_HPP_
