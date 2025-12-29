#ifndef __ALKOS_KERNEL__

#include "platform.h"
#include "sys/calls.h"

DEFINE_SYSCALL_VOID(
    get_clock_value, kSysGetClockValue, const ClockType, type, TimeVal *, time, Timezone *,
    time_zone
)
DEFINE_SYSCALL(get_clock_ticks_in_second, kSysGetClockTicksInSecond, u64, const ClockType, type)
DEFINE_SYSCALL_VOID(get_timezone, kSysGetTimezone, Timezone *, time_zone)

DEFINE_SYSCALL_VOID(debug_write, kSysDebugWrite, const char *, buffer)
DEFINE_SYSCALL(debug_read_line, kSysDebugReadLine, size_t, char *, buff, size_t, size)
DEFINE_SYSCALL_VOID(write_console, kSysWriteConsole, const char *, buffer)

DEFINE_SYSCALL_VOID(panic, kSysPanic, const char *, msg)

#endif  // __ALKOS_KERNEL__
