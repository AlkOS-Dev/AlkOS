#include <time.h>
#include <test_module/test_module.hpp>

/* internal includes */
#include <assert.h>
#include <extensions/debug.hpp>
#include "init.hpp"
#include "terminal.hpp"

static void KernelRun()
{
    static constexpr size_t kBuffSize = 256;
    char buff[kBuffSize];

    const auto t = time(nullptr);
    strftime(buff, kBuffSize, "Today we have: %Y-%m-%d %H:%M:%S\n", localtime(&t));

    arch::TerminalWriteString("Hello from AlkOS!\n");
    arch::TerminalWriteString(buff);
}

extern "C" void KernelMain()
{
    TRACE_INFO("Running kernel initialization...");
    KernelInit();

    if constexpr (kIsAlkosTestBuild) {
        TRACE_INFO("Running tests...");
        test::TestModule test_module{};
        test_module.RunTestModule();
        R_ASSERT(false && "Test module should never exit!");
    }

    TRACE_INFO("Proceeding to KernelRun...");
    KernelRun();
}
