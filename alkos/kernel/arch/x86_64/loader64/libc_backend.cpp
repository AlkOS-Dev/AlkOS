// This file provides the loader's implementation of the libc platform ABI.
#include <platform.h>

#include <autogen/feature_flags.h> 
#include <assert.h>
#include <arch_utils.hpp> 
#include <terminal.hpp>   

void __platform_panic(const char* msg)
{
    arch::TerminalWriteError("[LOADER LIBC PANIC]: ");
    arch::TerminalWriteError(msg);
    arch::TerminalWriteError("\n");

    OsHang();
}

void __platform_get_clock_value(ClockType /*type*/, TimeVal* time, Timezone* time_zone)
{
    if (time != nullptr) {
        time->seconds = 0;
        time->remainder = 0;
    }

    if (time_zone != nullptr) {
        time_zone->west_offset_minutes = 0;
        time_zone->dst_time_offset_minutes = 0;
        time_zone->dst_time_start_seconds = 0;
        time_zone->dst_time_end_seconds = 0;
    }
}

u64 __platform_get_clock_ticks_in_second(ClockType /*type*/)
{
    return 1;
}

void __platform_get_timezone(Timezone* time_zone)
{
    if (time_zone != nullptr) {
        time_zone->west_offset_minutes = 0;
        time_zone->dst_time_offset_minutes = 0;
        time_zone->dst_time_start_seconds = 0;
        time_zone->dst_time_end_seconds = 0;
    }
}

void __platform_debug_write(const char* buffer)
{
    arch::TerminalWriteString(buffer);
}

size_t __platform_debug_read_line(char* buffer, size_t buffer_size)
{
    return arch::TerminalReadLine(buffer, buffer_size);
}
