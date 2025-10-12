#include <assert.h>
#include <autogen/feature_flags.h>
#include <time.h>
#include <extensions/debug.hpp>
#include <test_module/test_module.hpp>
#include <trace.hpp>
/* internal includes */
#include "hal/terminal.hpp"
#include "init.hpp"
#include "todo.hpp"

static void KernelRun()
{
    KernelTraceSuccess("Hello from AlkOS!");
    TODO_MMU_MINIMAL
    // static constexpr size_t kBuffSize = 256;
    // char buff[kBuffSize];
    //
    // const auto t = time(nullptr);
    // strftime(buff, kBuffSize, "%Y-%m-%d %H:%M:%S", localtime(&t));
    //
    // KernelTraceSuccess("Hello from AlkOS! Today we have: %s", buff);
}

extern "C" void KernelMain()
{
    KernelTraceInfo("Running kernel initialization...");
    KernelInit();

    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        KernelTraceInfo("Running tests...");
        test::TestModule test_module{};
        test_module.RunTestModule();
        R_ASSERT(false && "Test module should never exit!");
    }

    TRACE_INFO("Proceeding to KernelRun...");
    KernelRun();
}
