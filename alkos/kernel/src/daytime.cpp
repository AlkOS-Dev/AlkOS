#include <extensions/debug.hpp>
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
    const timezone& tz = GetSetting<global_state_constants::SettingsType::kIsDayTimeClockInUTC>()
                             ? timing_constants::kUtcTimezone
                             : GetTimezone();

    time_ = QuerySystemTime(tz);
    TRACE_INFO("Synced system time with hardware: %lu", time_);
}
