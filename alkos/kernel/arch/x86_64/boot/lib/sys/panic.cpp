#include "sys/panic.hpp"
#include "cpu/utils.hpp"
#include "sys/terminal.hpp"

extern "C" void NO_RET KernelPanic(const char *msg)
{
    TerminalWriteError("[ KERNEL PANIC ]\n");
    TerminalWriteError(msg);
    TerminalWriteError("\n");
    OsHang();
}
