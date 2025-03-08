#include <test_module/test_module.hpp>

/* internal includes */
#include <assert.h>
#include <debug.hpp>
#include <loader64_main.hpp>
#include <terminal.hpp>

static void KernelRun() { TerminalWriteString("Hello from AlkOS!\n"); }

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
