#include <autogen/feature_flags.h>
#include <time.h>
#include <test_module/test_module.hpp>

/* internal includes */
#include <assert.h>
#include <extensions/debug.hpp>
#include <trace.hpp>
#include "init.hpp"
#include "terminal.hpp"

static void KernelRun()
{
    static constexpr size_t kBuffSize = 256;
    char buff[kBuffSize];

    const auto t = time(nullptr);
    strftime(buff, kBuffSize, "%Y-%m-%d %H:%M:%S", localtime(&t));

    KernelTraceSuccess("Hello from AlkOS! Today we have: %s", buff);
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
