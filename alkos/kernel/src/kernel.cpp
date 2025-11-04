#include <assert.h>
#include <autogen/feature_flags.h>
#include <time.h>
#include <test_module/test_module.hpp>
#include "trace_framework.hpp"

/* internal includes */
#include <hal/debug.hpp>

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "mem/heap.hpp"
#include "todo.hpp"

extern void KernelInit(const hal::RawBootArguments &);

static void KernelRun()
{
    TRACE_INFO_GENERAL("Hello from AlkOS!");
    trace::DumpAllBuffersOnFailure();
    TODO_MMU_MINIMAL
    // static constexpr size_t kBuffSize = 256;
    // char buff[kBuffSize];
    //
    // const auto t = time(nullptr);
    // strftime(buff, kBuffSize, "%Y-%m-%d %H:%M:%S", localtime(&t));
    //
    // KernelTraceSuccess("Hello from AlkOS! Today we have: %s", buff);
}

extern "C" void KernelMain(const hal::RawBootArguments *raw_args)
{
    ASSERT_NOT_NULL(raw_args, "Raw boot arguments are null");
    TRACE_INFO_GENERAL("Running kernel initialization...");

    hal::DebugStack();
    KernelInit(*raw_args);

    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        TRACE_INFO_GENERAL("Running tests...");
        test::TestModule test_module{};
        test_module.RunTestModule();
        R_ASSERT(false && "Test module should never exit!");
    }

    TRACE_INFO_GENERAL("Proceeding to KernelRun...");
    KernelRun();
}
