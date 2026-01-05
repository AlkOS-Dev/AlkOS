#include <assert.h>
#include <autogen/feature_flags.h>
#include <test_module/test_module.hpp>

/* internal includes */
#include "hal/boot_args.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "scheduling/kworker.hpp"
#include "trace_framework.hpp"

extern void KernelInit(const hal::RawBootArguments &);

static void KernelRun()
{
    TRACE_INFO_GENERAL("Hello from AlkOS!");
    const auto res = SchedulingModule::Get().GetTaskMgr().ExecuteElf64("/bin/shell", {});
    R_ASSERT_TRUE(static_cast<bool>(res), "Failed to spawn /bin/shell process...");

    SchedulingModule::Get().GetScheduler().ConvertToScheduling();
}

extern "C" void KernelMain(const Mem::PPtr<hal::RawBootArguments> raw_args)
{
    ASSERT_NOT_NULL(raw_args, "Raw boot arguments are null");

    KernelInit(*Mem::PhysToVirt(raw_args));

    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        TRACE_INFO_GENERAL("Running tests...");
        test::TestModule test_module{};
        test_module.RunTestModule();
        R_ASSERT(false && "Test module should never exit!");
    }

    TRACE_INFO_GENERAL("Proceeding to KernelRun...");
    KernelRun();
}
