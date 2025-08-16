#include <extensions/debug.hpp>
#include <modules/global_state.hpp>
#include <time.hpp>
#include <time/daytime.hpp>

using namespace timing;

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

    const timezone &tz = GlobalStateModule::Get()
                                 .GetSettings()
                                 .Get<global_state_constants::SettingsType::kIsDayTimeClockInUTC>()
                             ? kUtcTimezone
                             : GetTimezone();

    time_ = arch::QuerySystemTime(tz);
    TRACE_INFO("Synced system time with hardware: %lu, %s", time_, [&] {
        tm time;
        strftime(
            buffer, kBuffSize, "%Y-%m-%d %H:%M:%S", ConvertFromPosixToTm(time_, time, GetTimezone())
        );
        return buffer;
    }());
}
