/// This file provides the kernel's implementation of the libc platform ABI.
#ifdef __ALKOS_KERNEL__
#include <assert.h>
#include <platform.h>

#include "hal/debug_terminal.hpp"
#include "hal/panic.hpp"
#include "hal/spinlock.hpp"
#include "modules/timing.hpp"
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

fd_t __platform_open(const char *pathname, const int flags)
{
    auto res = SysOpen(vfs::Path(pathname), static_cast<Fs::OpenMode>(flags));
    if (!res) {
        return -1;
    }

    return *res;
}

int __platform_close(const fd_t fd)
{
    auto res = SysClose(fd);
    if (!res) {
        return -1;
    }

    return 0;
}

ssize_t __platform_read(const fd_t fd, void *buf, const size_t count)
{
    auto res = SysRead(fd, {static_cast<byte *>(buf), count});
    if (!res) {
        DEBUG_FATAL_VFS("Read syscall failed on fd %d: %d", fd, static_cast<int>(res.error()));
        return -1;
    }
    return static_cast<ssize_t>(*res);
}

ssize_t __platform_write(const fd_t fd, const void *buf, const size_t count)
{
    auto res = SysWrite(fd, {static_cast<const byte *>(buf), count});
    if (!res) {
        return -1;
    }
    return static_cast<ssize_t>(*res);
}

ssize_t __platform_seek(int fd, ssize_t offset, FdSeek whence)
{
    auto res = SysSeek(fd, offset, static_cast<Fs::FdSeek>(whence));
    if (!res) {
        return -1;
    }
    return *res;
}

#endif
