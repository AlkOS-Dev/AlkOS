#include <autogen/feature_flags.h>
#include "cpu/utils.hpp"
#ifndef __i386__
#include <test_module/test_module.hpp>
#endif

#include "hal/impl/panic.hpp"
#include <hal/impl/terminal.hpp>

namespace arch {

extern "C" void NO_RET KernelPanic(const char *msg)
{
    arch::TerminalWriteError("[ KERNEL PANIC ]\n");
    arch::TerminalWriteError(msg);
    arch::TerminalWriteError("\n");

#ifndef __i386__
    /* Tests are supposed to run only in x86_64 target */

    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
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

}
