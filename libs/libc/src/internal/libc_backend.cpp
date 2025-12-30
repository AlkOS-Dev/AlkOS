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

DEFINE_SYSCALL(open, kSysOpen, int, const char *, pathname, int, flags)
DEFINE_SYSCALL(close, kSysClose, int, int, fd)
DEFINE_SYSCALL(read, kSysRead, ssize_t, int, fd, void *, buf, size_t, count)
DEFINE_SYSCALL(write, kSysWrite, ssize_t, int, fd, const void *, buf, size_t, count)
DEFINE_SYSCALL(seek, kSysSeek, ssize_t, int, fd, ssize_t, offset, FdSeek, whence)

#endif  // __ALKOS_KERNEL__
