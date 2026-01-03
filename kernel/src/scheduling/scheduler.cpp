#include "scheduling/scheduler.hpp"

#include "autogen/feature_flags.h"
#include "hal/scheduling.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"

namespace Sched
{
Scheduler::Scheduler()
{
    policies_[static_cast<size_t>(SchedulingPolicy::kUberTask_PQ_P0)] =
        PreparePolicy<PriorityQueuePolicy>(&policy0_);
    policies_[static_cast<size_t>(SchedulingPolicy::kDrivers_PQ_P1)] =
        PreparePolicy<PriorityQueuePolicy>(&policy1_);
    policies_[static_cast<size_t>(SchedulingPolicy::kUrgentTasks_PQ_P2)] =
        PreparePolicy<PriorityQueuePolicy>(&policy2_);
    policies_[static_cast<size_t>(SchedulingPolicy::kNormalTasks_RR_P3)] =
        PreparePolicy<RoundRobinPolicy>(&policy3_);
    policies_[static_cast<size_t>(SchedulingPolicy::kBackgroundTasks_RR_P4)] =
        PreparePolicy<RoundRobinPolicy>(&policy4_);
}

void Scheduler::AddReadyThread(Thread *thread)
{
    const auto idx = static_cast<size_t>(thread->flags.policy);
    ASSERT_LT(idx, static_cast<size_t>(SchedulingPolicy::kLast));

    const auto policy = policies_[idx];
    policy.cbs.add_task(policy.self, thread);
}

Thread *Scheduler::Schedule()
{
    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        return nullptr;
    }

    for (const auto &policy : policies_) {
        if (const auto thread = policy.cbs.pick_next_task(policy.self)) {
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
