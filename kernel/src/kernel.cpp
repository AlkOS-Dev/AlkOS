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

    auto &task_mgr = SchedulingModule::Get().GetTaskMgr();

    auto res = task_mgr.ExecuteElf64("/bin/hello", {});
    R_ASSERT_TRUE(res, "Failed to spawn /bin/hello process...");
    auto [pid, _] = res.value();

    Sched::Task tracer_task;
    tracer_task.func       = reinterpret_cast<void *>(Sched::StdoutTracerMain);
    tracer_task.args_count = 1;
    tracer_task.args       = {*reinterpret_cast<u64 *>(&pid)};

    // Spawn StdoutTracer KWorker to monitor hello_world's stdout
    R_ASSERT_TRUE(
        task_mgr.SpawnKernelProcess("stdout-tracer", {}, tracer_task),
        "Failed to spawn stdout tracer process..."
    );

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
