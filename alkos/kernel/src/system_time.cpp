#include <extensions/debug.hpp>
#include <modules/global_state.hpp>
#include <time.hpp>
#include <time/system_time.hpp>

using namespace timing;

// ------------------------------
// Implementations
// ------------------------------

SystemTime::SystemTime()
{
    TRACE_INFO("DayTime::DayTime()");
    SyncWithHardware();
}

void SystemTime::SyncWithHardware()
{
    static constexpr size_t kBuffSize = 64;
    [[maybe_unused]] char buffer[kBuffSize];
    boot_time_read_utc_ = arch::QuerySystemTime(kUtcTimezone);
    TRACE_INFO("Synced system time with hardware: %lu, %s", boot_time_read_utc_, [&] {
        tm time;
        strftime(
            buffer, kBuffSize, "%Y-%m-%d %H:%M:%S",
            ConvertFromPosixToTm(boot_time_read_utc_, time, GetTimezone())
        );
        return buffer;
    }());
}
