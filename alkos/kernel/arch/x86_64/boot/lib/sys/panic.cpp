/* internal includes */
#include <cpu/utils.hpp>
#include <sys/panic.hpp>
#include <sys/terminal.hpp>

extern "C" void NO_RET KernelPanic(const char *msg)
{
    arch::TerminalWriteError("[ KERNEL PANIC ]\n");
    arch::TerminalWriteError(msg);
    arch::TerminalWriteError("\n");
    OsHang();
}
