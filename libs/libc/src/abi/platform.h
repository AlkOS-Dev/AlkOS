#ifndef LIBS_LIBC_SRC_ABI_PLATFORM_H_
#define LIBS_LIBC_SRC_ABI_PLATFORM_H_

#include "defines.h"
#include "types.h"

#include "alkos/structs.h"
#include "alkos/syscall.h"

BEGIN_DECL_C

SYSCALL_VOID_NAME(
    get_clock_value, kSysGetClockValue, const ClockType, type, TimeVal *, time, Timezone *,
    time_zone
);
SYSCALL_NAME(get_clock_ticks_in_second, kSysGetClockTicksInSecond, u64, const ClockType, type);
SYSCALL_VOID_NAME(get_timezone, kSysGetTimezone, Timezone *, time_zone);

SYSCALL_VOID_NAME(debug_write, kSysDebugWrite, const char *, buffer);
SYSCALL_NAME(debug_read_line, kSysDebugReadLine, size_t, char *, buff, size_t, size);

SYSCALL_VOID_NAME(panic, kSysPanic, const char *, msg);

SYSCALL_NAME(open, kSysOpen, int, const char *, pathname, int, flags);
SYSCALL_NAME(close, kSysClose, int, fd_t, fd);
SYSCALL_NAME(read, kSysRead, ssize_t, fd_t, fd, void *, buf, size_t, count);
SYSCALL_NAME(write, kSysWrite, ssize_t, fd_t, fd, const void *, buf, size_t, count);
SYSCALL_NAME(seek, kSysSeek, ssize_t, fd_t, fd, ssize_t, offset, FdSeek, whence);
SYSCALL_NAME(dup, kSysDup, fd_t, fd_t, fd);
SYSCALL_NAME(dup_to, kSysDupTo, fd_t, fd_t, old_fd, fd_t, new_fd);
SYSCALL_NAME(
    read_directory, kSysReadDirectory, int, const char *, path, DirEntry *, entries, size_t,
    max_entries, size_t *, num_entries
);
SYSCALL_NAME(file_info, kSysFileInfo, int, const char *, path, FileInfo *, info);

/* Thread, processes */
SYSCALL_NAME(thread_create, kThreadCreate, int, Thread *, thread, thread_func_t, f, void *, arg);
SYSCALL_NAME(exec, kExec, int, const char *, path, u64 *, pid);
SYSCALL_VOID_NAME(thread_exit, kThreadExit, void *, retval);
SYSCALL_NAME(thread_join, kThreadJoin, int, Thread *, thread, void **, retval);
SYSCALL_NAME(thread_detach, kThreadDetach, int, Thread *, thread);
SYSCALL_VOID_NAME(proc_exit, kProcExit, int, status);
SYSCALL_VOID_NAME(proc_abort, kProcAbort);
SYSCALL_VOID_NAME(nanosleep, kNanoSleep, u64, time_ns);
SYSCALL_VOID_NAME(nanosleep_until, kNanoSleepUntil, u64, systime_ns);
SYSCALL_NAME(kill, kKill, int, u64, pid);
SYSCALL_NAME(wait, kWait, int, u64, pid);

// Video
SYSCALL_VOID_NAME(create_graphic_session, kSysCreateGraphicSession, GuiBufferInfo *, info);
SYSCALL_VOID_NAME(blit, kSysBlit);

// Power Management
SYSCALL_VOID_NAME(power, kSysPower, PowerAction, action);

END_DECL_C

#endif  // LIBS_LIBC_SRC_ABI_PLATFORM_H_
