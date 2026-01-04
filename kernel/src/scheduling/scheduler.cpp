#include "scheduling/scheduler.hpp"

#include "autogen/feature_flags.h"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"
#include "scheduling/scheduler_lock.hpp"

// ------------------------------
// statics
// ------------------------------

static Sched::Thread *TimerHandler(intr::LitHwEntry &)
{
    if (hardware::GetCoreLocalTcb() == nullptr) {
        // Not yet converted to scheduling
        return nullptr;
    }

    // TODO: Idle

    // 1.

    return SchedulingModule::Get().GetScheduler().ScheduleAndUpdateThreads();
}

// ------------------------------
// Implementations
// ------------------------------

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

void Scheduler::InstallInterruptHandler()
{
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
            hal::kTimerHwLirq, intr::HwHandler{.handler = TimerHandler}
        );
}

void Scheduler::AddReadyThread(Thread *thread)
{
    ASSERT_EQ(thread->state, ThreadState::kReady);

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
    return nullptr;
}

Thread *Scheduler::ScheduleAndUpdateThreads()
{
    const auto thread = Schedule();
    ASSERT_EQ(thread->state, ThreadState::kReady);
    thread->state = ThreadState::kRunning;

    ASSERT_NOT_NULL(hardware::GetCoreLocalTcb());
    ASSERT_EQ(hardware::GetCoreLocalTcb()->state, ThreadState::kRunning);
    hardware::GetCoreLocalTcb()->state = ThreadState::kReady;
    AddReadyThread(hardware::GetCoreLocalTcb());

    PrepareNextTimerInterruptBeforeSwitchUnguarded_(thread);

    return thread;
}

void Scheduler::Yield()
{
    SchedulerLock lock{};
    YieldUnguarded();
}

void Scheduler::ConvertToScheduling()
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();

    ASSERT_TRUE(HardwareModule::Get().GetEventClockRegistry().IsSelectedPicked());

    auto event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    ASSERT_TRUE(event_clock.flags.IsCoreLocal, "Scheduler supports only core local event clocks");
    event_clock.cbs.set_oneshot(&event_clock);

    const auto thread = Schedule();
    ASSERT_EQ(thread->state, ThreadState::kReady);
    thread->state = ThreadState::kRunning;

    const auto &policy        = GetPolicy_(thread);
    const u64 preempt_time_ns = policy.cbs.get_preempt_time(policy.self, thread);
    ASSERT_GT(preempt_time_ns, kMinDelta);

    SetupNextTimeEvent_(preempt_time_ns);
    hal::ConvertContext(thread);
}

void Scheduler::NanoSleepUntil(const u64 systime_ns)
{
    u64 time;
    {
        SchedulerLock lock{};
        time = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
        if (systime_ns < time) {
            return;
        }
    }

    if (systime_ns - time < kMinDelta) {
        /* Do busy wait if window to scheduler irq is too small */
        while (systime_ns - TimingModule::Get().GetSystemTime().ReadLifeTimeNs() < kMinDelta) {
        }
        return;
    }

    {
        SchedulerLock lock{};

        hardware::GetCoreLocalTcb()->key   = systime_ns;
        hardware::GetCoreLocalTcb()->state = ThreadState::kSleeping;
        sleep_queue_.Insert(hardware::GetCoreLocalTcb());

        const auto thread = Schedule();
        ASSERT_EQ(thread->state, ThreadState::kReady);

        PrepareNextTimerInterruptBeforeSwitchUnguarded_(thread);

        thread->state = ThreadState::kRunning;
        hal::ContextSwitch(thread);
    }
}

void Scheduler::PrepareNextTimerInterruptBeforeSwitchUnguarded_(Thread *next_thread)
{
    ASSERT_NOT_NULL(next_thread);
    u64 min_time_ns = std::numeric_limits<u64>::max();

    /* Empty all tasks that are too early to schedule timer on */
    u64 time{};
    while (!sleep_queue_.IsEmpty()) {
        time                 = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
        const u64 sleep_time = sleep_queue_.Min()->key;

        if (sleep_time > time && sleep_time - time > kMinDelta) {
            break;
        }

        const auto thread = sleep_queue_.Min();
        sleep_queue_.Delete(thread);
        ASSERT_EQ(thread->state, ThreadState::kSleeping);

        thread->state = ThreadState::kReady;
        AddReadyThread(thread);
    }

    if (!sleep_queue_.IsEmpty()) {
        ASSERT_NOT_ZERO(time);

        min_time_ns = sleep_queue_.Min()->key - time;
    }

    const auto &policy     = GetPolicy_(next_thread);
    const u64 preempt_time = policy.cbs.get_preempt_time(policy.self, next_thread);
    ASSERT_GT(preempt_time, kMinDelta);

    min_time_ns = std::min(preempt_time, min_time_ns);

    ASSERT_GT(min_time_ns, kMinDelta);
    SetupNextTimeEvent_(min_time_ns);
}

void Scheduler::SetupNextTimeEvent_(const u64 time_ns)
{
    ASSERT_TRUE(HardwareModule::Get().GetEventClockRegistry().IsSelectedPicked());

    auto event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    ASSERT_TRUE(event_clock.flags.IsCoreLocal, "Scheduler supports only core local event clocks");

    event_clock.cbs.next_event(&event_clock, time_ns);
}

}  // namespace Sched
