#include "syscalls.hpp"
#include "dispatch_table.hpp"

#include <alkos/syscall.h>

using namespace Syscall;

BEGIN_DECL_C

size_t g_syscall_count                  = kSysMax;
constinit auto g_syscall_dispatch_table = SyscallDispatchTable<kSysMax>::Create<[] {
    SyscallDispatchTable<kSysMax> table{};

    // Time related syscalls
    table.RegisterHandler<kSysGetClockValue, SysGetClockValue>();
    table.RegisterHandler<kSysGetTimezone, SysGetTimezone>();
    table.RegisterHandler<kSysGetClockTicksInSecond, SysGetClockTicksInSecond>();

    // Debug IO syscalls
    table.RegisterHandler<kSysDebugWrite, SysDebugWrite>();
    table.RegisterHandler<kSysDebugReadLine, SysDebugReadLine>();
    table.RegisterHandler<kSysWriteConsole, SysWriteConsole>();

    // Panic syscall
    table.RegisterHandler<kSysPanic, SysPanic>();

    // File Descriptor syscalls
    table.RegisterHandler<kSysOpen, SysOpen>();
    table.RegisterHandler<kSysClose, SysClose>();
    table.RegisterHandler<kSysRead, SysRead>();
    table.RegisterHandler<kSysWrite, SysWrite>();
    table.RegisterHandler<kSysSeek, SysSeek>();

    /* Thread, processes */

    return table;
}>();

END_DECL_C
