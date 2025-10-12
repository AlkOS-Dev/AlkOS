/// This file provides the kernel's implementation of the libc platform ABI.
#include <assert.h>
#include <platform.h>

#include "hal/debug_terminal.hpp"
#include "hal/panic.hpp"
#include "modules/timing.hpp"
#include "modules/timing_constants.hpp"

void __platform_panic(const char* msg) { hal::KernelPanic(msg); }

void __platform_get_clock_value(const ClockType type, TimeVal* time, Timezone* time_zone)
{
    ASSERT_NOT_NULL(time);
    ASSERT_NOT_NULL(time_zone);

    if (!TimingModule::IsInited()) {
        time->seconds   = 0;
        time->remainder = 0;
        return;
    }

    if (time_zone != nullptr) {
        __platform_get_timezone(time_zone);
    }

    if (time != nullptr) {
        switch (type) {
            case kTimeUtc: {
                time->seconds   = TimingModule::Get().GetSystemTime().Read();
                time->remainder = 0;
            } break;
            case kProcTime: {  // In microseconds
                time->seconds   = 0;
                time->remainder = timing::SystemTime::ReadLifeTimeNs() / 1000;
            } break;
            case kProcTimePrecise: {  // In nanoseconds
                time->seconds   = 0;
                time->remainder = timing::SystemTime::ReadLifeTimeNs();
            } break;
            default:
                R_FAIL_ALWAYS("Provided invalid ClockType!");
        }
    }
}

u64 __platform_get_clock_ticks_in_second(const ClockType type)
{
    const auto idx = static_cast<size_t>(type);
    ASSERT(idx != 0 && idx < timing_constants::kClockTicksInSecondSize);

    TODO_USERSPACE
    //    if (idx == 0 || idx >= kResoSize) {
    //        return 0;
    //    }

    return timing_constants::kClockTicksInSecond[idx];
}

void __platform_get_timezone(Timezone* time_zone)
{
    assert(time_zone != nullptr);
    *time_zone = TimingModule::Get().GetSystemTime().GetTimezone();
}

void __platform_debug_write(const char* buffer) { hal::DebugTerminalWrite(buffer); }

size_t __platform_debug_read_line(char* buffer, const size_t buffer_size)
{
    return hal::DebugTerminalReadLine(buffer, buffer_size);
}
