/// This file provides the kernel's implementation of the libc platform ABI.
#ifdef __ALKOS_KERNEL__

#include <assert.h>
#include <platform.h>

#include "hal/panic.hpp"
#include "modules/timing_constants.hpp"
#include "syscalls/syscalls.hpp"

using namespace Syscall;

void __platform_panic(const char *msg) { SysPanic(msg); }

void __platform_get_clock_value(const ClockType type, TimeVal *time, Timezone *time_zone)
{
    SysGetClockValue(type, time, time_zone);
}

u64 __platform_get_clock_ticks_in_second(const ClockType type)
{
    return SysGetClockTicksInSecond(type);
}

void __platform_get_timezone(Timezone *time_zone) { SysGetTimezone(time_zone); }

void __platform_debug_write(const char *buffer) { SysDebugWrite(buffer); }

size_t __platform_debug_read_line(char *buffer, const size_t buffer_size)
{
    return SysDebugReadLine(buffer, buffer_size);
}

void __platform_write_console(const char *buffer) { SysWriteConsole(buffer); }

#endif
