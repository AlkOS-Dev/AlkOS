#include <assert.h>

#include <test_module/test.hpp>

#include "hal/debug.hpp"
#include "hal/scheduling.hpp"
#include "hardware/core_local.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "trace_framework.hpp"

MTEST(AssertSnprintfTest) { ASSERT_EQ(2 + 2, 3 + 3, "Values used in the test: %d, %d", 2, 3); }

// ------------------------------
// KernelTaskSwitchTest
// ------------------------------

static void Task0()
{
    while (true) {
        TRACE_INFO_GENERAL("Ack from Task0!");

        static constexpr size_t kIters = 1'000'000;
        for (size_t i = 0; i < kIters; i++) {
            hal::Noop();
            hal::Noop();
            hal::Noop();
            hal::Noop();
        }

        const auto tcb = hardware::GetCurrentTCB();
        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
        hal::ContextSwitch(tcb->next);
    }
}

static void Task1()
{
    while (true) {
        TRACE_INFO_GENERAL("Ack from Task1!");

        static constexpr size_t kIters = 1'000'000;
        for (size_t i = 0; i < kIters; i++) {
            hal::Noop();
            hal::Noop();
            hal::Noop();
            hal::Noop();
        }

        const auto tcb = hardware::GetCurrentTCB();

        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
        hal::ContextSwitch(tcb->next);
    }
}

MTEST(KernelTaskSwitchTest)
{
    auto p0 = SchedulingModule::Get().GetTaskMgr().SpawnProcess(Task0, true);
    ASSERT_TRUE(static_cast<bool>(p0));
    auto p1 = SchedulingModule::Get().GetTaskMgr().SpawnProcess(Task1, true);
    ASSERT_TRUE(static_cast<bool>(p1));

    const auto tid0 = std::get<1>(p0.value());
    const auto tid1 = std::get<1>(p1.value());

    const auto t0 = SchedulingModule::Get().GetThreads().GetThread(tid0);
    ASSERT_TRUE(static_cast<bool>(t0));
    const auto t1 = SchedulingModule::Get().GetThreads().GetThread(tid1);
    ASSERT_TRUE(static_cast<bool>(t1));

    t0.value()->next = t1.value();
    t1.value()->next = t0.value();

    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    hal::ConvertContext(t0.value());
}
