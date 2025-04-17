/* internal includes */
#include <arch_utils.hpp>
#include <panic.hpp>
#include <terminal.hpp>
#ifndef __i386__
#include <test_module/test_module.hpp>
#endif

extern "C" void NO_RET KernelPanic(const char *msg)
{
    arch::TerminalWriteError("[ KERNEL PANIC ]\n");
    arch::TerminalWriteError(msg);
    arch::TerminalWriteError("\n");

#ifndef __i386__
    /* Tests are supposed to run only in x86_64 target */

    if constexpr (kIsAlkosTestBuild) {
        /* When running tests */
        test::OnKernelPanic();
    } else {
        /* Usual situation */
        OsHang();
    }
#else
    OsHang();
#endif
}
