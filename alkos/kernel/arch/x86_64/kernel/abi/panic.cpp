/* internal includes */
#include <arch_utils.hpp>
#include <panic.hpp>
#include <terminal.hpp>
#ifndef __i386__
#include <test_module/test_module.hpp>
#endif

extern "C" void NO_RET KernelPanic(const char *msg)
{
    TerminalWriteError("[ KERNEL PANIC ]\n");
    TerminalWriteError(msg);
    TerminalWriteError("\n");

#if defined(__i386__)
    OsHang();
#else
    if constexpr (kIsAlkosTestBuild) {
        test::OnKernelPanic();
    } else {
        OsHang();
    }
#endif
}
