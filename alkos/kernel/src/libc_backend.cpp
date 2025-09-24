/// This file provides the kernel's implementation of the libc platform ABI.
#include <assert.h>
#include <platform.h>

#include "debug_terminal.hpp"
#include "modules/timing.hpp"
#include "modules/timing_constants.hpp"
#include "panic.hpp"

void __platform_panic(const char* msg) { arch::KernelPanic(msg); }

void __platform_get_clock_value(ClockType type, TimeVal* time, Timezone* time_zone)
{
    assert(time != nullptr || time_zone != nullptr);

    if (time_zone != nullptr) {
        __platform_get_timezone(time_zone);
    }

    if (time != nullptr) {
        switch (type) {
            case kTimeUtc: {
                time->seconds   = TimingModule::Get().GetDayTime().GetTime();
                time->remainder = 0;
            } break;
            case kProcTime: {
                R_FAIL_ALWAYS("Not implemented yet!");
            } break;
            default:
                R_FAIL_ALWAYS("Provided invalid ClockType!");
        }
    }
}

u64 __platform_get_clock_ticks_in_second(ClockType type)
{
    const size_t idx = static_cast<size_t>(type);
    ASSERT(idx != 0 && idx < timing_constants::kClockTicksInSecondSize);
    return timing_constants::kClockTicksInSecond[idx];
}

void __platform_get_timezone(Timezone* time_zone)
{
    assert(time_zone != nullptr);
    *time_zone = TimingModule::Get().GetDayTime().GetTimezone();
}

void __platform_debug_write(const char* buffer) { DebugTerminalWrite(buffer); }

size_t __platform_debug_read_line(char* buffer, size_t buffer_size)
{
    return DebugTerminalReadLine(buffer, buffer_size);
}
