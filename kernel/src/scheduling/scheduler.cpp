#include "scheduling/scheduler.hpp"

#include "autogen/feature_flags.h"
#include "hal/scheduling.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"

namespace Sched
{
void Scheduler::AddReadyThread(Thread *thread)
{
    // if (threads_ == nullptr) {
    //     threads_     = thread;
    //     thread->next = thread;
    //     return;
    // }
    //
    // thread->next   = threads_->next;
    // threads_->next = thread;
    // threads_       = thread;
}

Thread *Scheduler::Schedule()
{
    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        return nullptr;
    }

    for (const auto &policy : policies_) {
        if (const auto thread = policy.cbs.pick_thread(policy.self)) {
            return thread;
        }
    }

    // TODO: IDLE
    // TODO: self pick
    return nullptr;
}
void Scheduler::Yield()
{
    auto thread = Schedule();

    if (thread != nullptr) {
        hal::ContextSwitch(thread);
    }
}

void Scheduler::ConvertToScheduling()
{
    // HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    //
    // static constexpr u64 kPeriodicTime1Ms = kNanosInSecond / 1'000;
    // TimingModule::Get().GetEventFramework().SetupPeriodic(kPeriodicTime1Ms);
    //
    // ASSERT_NOT_NULL(threads_);
    //
    // auto thread = GetNext_();
    // ASSERT_NOT_NULL(thread);
    //
    // auto owner = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    // ASSERT_TRUE(static_cast<bool>(owner));
    //
    // hal::ConvertContext(thread);
}
}  // namespace Sched
