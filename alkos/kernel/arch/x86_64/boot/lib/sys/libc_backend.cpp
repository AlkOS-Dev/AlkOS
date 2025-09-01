// This file provides the loader's implementation of the libc platform ABI.
#include <platform.h>

#include <assert.h>
#include <autogen/feature_flags.h>
#include "cpu/utils.hpp"
#include "hw/serial/qemu.hpp"
#include "panic.hpp"

void __platform_panic(const char* msg)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        KernelPanic(msg);
    }

    OsHang();
}

void __platform_get_clock_value(ClockType /*type*/, TimeVal* time, Timezone* time_zone)
{
    if (time != nullptr) {
        time->seconds   = 0;
        time->remainder = 0;
    }

    if (time_zone != nullptr) {
        time_zone->west_offset_minutes     = 0;
        time_zone->dst_time_offset_minutes = 0;
        time_zone->dst_time_start_seconds  = 0;
        time_zone->dst_time_end_seconds    = 0;
    }
}

u64 __platform_get_clock_ticks_in_second(ClockType /*type*/) { return 1; }

void __platform_get_timezone(Timezone* time_zone)
{
    if (time_zone != nullptr) {
        time_zone->west_offset_minutes     = 0;
        time_zone->dst_time_offset_minutes = 0;
        time_zone->dst_time_start_seconds  = 0;
        time_zone->dst_time_end_seconds    = 0;
    }
}

void __platform_debug_write(const char* buffer)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalWriteString(buffer);
    }
}

size_t __platform_debug_read_line(char* buffer, size_t buffer_size)
{
    /* Not implemented in loader */
    return 0;
}
