#include "time/system_time.hpp"

#include <extensions/debug.hpp>
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "hal/timers.hpp"

// ------------------------------
// Implementations
// ------------------------------

time_t timing::SystemTime::ReadLifeTimeNs()
{
    return HardwareModule::Get().GetClockRegistry().ReadTimeNsUnsafe();
}

void timing::SystemTime::SyncWithHardware()
{
    static constexpr size_t kBuffSize = 64;
    [[maybe_unused]] char buffer[kBuffSize];
    boot_time_read_utc_ = arch::QuerySystemTime(kUtcTimezone);
    sys_time_on_read_   = ReadLifeTimeNs();

    tm time;
    ConvertFromPosixToTm(boot_time_read_utc_, time, kUtcTimezone);
    boot_time_read_local_ = ConvertDateTimeToPosix(time, GetTimezone());

    TRACE_INFO("Synced system time with hardware: %lu, %s", boot_time_read_utc_, [&] {
        strftime(
            buffer, kBuffSize, "%Y-%m-%d %H:%M:%S",
            ConvertFromPosixToTm(boot_time_read_utc_, time, kUtcTimezone)
        );
        return buffer;
    }());
}
