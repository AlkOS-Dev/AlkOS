#include "syscalls.hpp"

#include <sys/calls.h>

using namespace Syscall;

BEGIN_DECL_C

size_t g_syscall_count                                  = kSysMax;
constinit SyscallDispatchTable g_syscall_dispatch_table = []() consteval {
    SyscallDispatchTable table{};

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

    return table;
}();

END_DECL_C
