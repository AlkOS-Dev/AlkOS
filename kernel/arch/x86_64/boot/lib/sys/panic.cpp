#include "alkos/panic.hpp"
#include "alkos/terminal.hpp"
#include "cpu/utils.hpp"

extern "C" void NO_RET KernelPanic(const char *msg)
{
    TerminalWriteError("[ KERNEL PANIC ]\n");
    TerminalWriteError(msg);
    TerminalWriteError("\n");
    OsHang();
}
