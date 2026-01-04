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

    // Panic syscall
    table.RegisterHandler<kSysPanic, SysPanic>();

    // File Descriptor syscalls
    table.RegisterHandler<kSysOpen, SysOpen>();
    table.RegisterHandler<kSysClose, SysClose>();
    table.RegisterHandler<kSysRead, SysRead>();
    table.RegisterHandler<kSysWrite, SysWrite>();
    table.RegisterHandler<kSysSeek, SysSeek>();
    table.RegisterHandler<kSysDup, SysDup>();
    table.RegisterHandler<kSysDupTo, SysDupTo>();
    table.RegisterHandler<kSysReadDirectory, SysReadDirectory>();
    table.RegisterHandler<kSysFileInfo, SysFileInfo>();

    /* Thread, processes */
    table.RegisterHandler<kProcAbort, SysAbort>();
    table.RegisterHandler<kProcExit, SysExit>();
    table.RegisterHandler<kThreadCreate, SysThreadCreate>();
    table.RegisterHandler<kThreadExit, SysThreadExit>();
    table.RegisterHandler<kThreadJoin, SysThreadJoin>();
    table.RegisterHandler<kThreadDetach, SysThreadDetach>();
    table.RegisterHandler<kNanoSleep, SysNanoSleep>();
    table.RegisterHandler<kNanoSleepUntil, SysNanoSleepUntil>();

    // Video
    table.RegisterHandler<kSysCreateGraphicSession, SysCreateGraphicSession>();
    table.RegisterHandler<kSysBlit, SysBlit>();

    // Power Management
    table.RegisterHandler<kSysPower, SysPower>();

    return table;
}>();

END_DECL_C
