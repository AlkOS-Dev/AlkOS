#include <extensions/debug.hpp>
#include <modules/global_state.hpp>
#include <time/daytime.hpp>
#include <timers.hpp>

USE_TIMING

// ------------------------------
// Implementations
// ------------------------------

DayTime::DayTime()
{
    TRACE_INFO("DayTime::DayTime()");
    SyncWithHardware();
}

void DayTime::SyncWithHardware()
{
    static constexpr size_t kBuffSize = 64;
    [[maybe_unused]] char buffer[kBuffSize];

    const timezone& tz = GetSetting<global_state_constants::SettingsType::kIsDayTimeClockInUTC>()
                             ? timing_constants::kUtcTimezone
                             : GetTimezone();

    time_ = QuerySystemTime(tz);
    TRACE_INFO("Synced system time with hardware: %lu, %s", time_, [&] {
        strftime(buffer, kBuffSize, "%Y-%m-%d %H:%M:%S\n", localtime(&time_));
        return buffer;
    }());
}
