/* internal includes */
#include <arch_utils.hpp>
#include <panic.hpp>
#include <terminal.hpp>

extern "C" void NO_RET KernelPanic(const char *msg)
{
    arch::TerminalWriteError("[ KERNEL PANIC ]\n");
    arch::TerminalWriteError(msg);
    arch::TerminalWriteError("\n");
    OsHang();
}
