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
    return SchedulingModule::Get().GetScheduler().TimerRoutine();
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

Thread *Scheduler::ScheduleAndUpdateThreads(const bool preempt, const ThreadState thread_state)
{
    u64 min_time_ns = std::numeric_limits<u64>::max();

    // 1. Wake up all tasks
    const bool force_preempt = WakeUpTasks();

    // 2. Check for sleepers time
    const u64 time = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    if (!sleep_queue_.IsEmpty()) {
        min_time_ns = sleep_queue_.Min()->key - time;
    }

    // 3. Scheduling new thread if needed and update structs
    Thread *thread{};
    u64 preempt_time_ns = GetPreemptTime_(hardware::GetCoreLocalTcb());
    if (preempt || force_preempt || ShouldPreempt_(preempt_time_ns)) {
        const auto next_thread = Schedule();
        ASSERT_EQ(next_thread->state, ThreadState::kReady);
        preempt_time_ns    = GetPreemptTime_(next_thread);
        next_thread->state = ThreadState::kRunning;

        ASSERT_NOT_NULL(hardware::GetCoreLocalTcb());
        ASSERT_EQ(hardware::GetCoreLocalTcb()->state, ThreadState::kRunning);
        hardware::GetCoreLocalTcb()->state = thread_state;

        if (thread_state == ThreadState::kReady) {
            AddReadyThread(hardware::GetCoreLocalTcb());
        }

        thread = next_thread;
    }

    // 4. Check with preempt time
    ASSERT_GT(preempt_time_ns, kMinDelta);
    min_time_ns = std::min(preempt_time_ns, min_time_ns);

    // 5. Schedule next timer
    ASSERT_GT(min_time_ns, kMinDelta);
    SetupNextTimeEvent_(min_time_ns);

    return thread;
}

void Scheduler::Yield()
{
    SchedulerLock lock{};
    hal::ContextSwitch(ScheduleAndUpdateThreads(true, ThreadState::kReady));
}

void Scheduler::ConvertToScheduling()
{
    TRACE_INFO_SCHEDULING("Converting to scheduling!");

    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();

    ASSERT_TRUE(HardwareModule::Get().GetEventClockRegistry().IsSelectedPicked());

    auto &event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    ASSERT_TRUE(event_clock.flags.IsCoreLocal, "Scheduler supports only core local event clocks");
    event_clock.cbs.set_oneshot(&event_clock);

    const auto thread = Schedule();
    ASSERT_EQ(thread->state, ThreadState::kReady);

    const u64 preempt_time_ns = GetPreemptTime_(thread);
    ASSERT_GT(preempt_time_ns, kMinDelta);

    thread->state = ThreadState::kRunning;
    SetupNextTimeEvent_(preempt_time_ns);

    TRACE_INFO_SCHEDULING("Converting to thread: %llu!", thread->tid);
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

        hardware::GetCoreLocalTcb()->key = systime_ns;
        sleep_queue_.Insert(hardware::GetCoreLocalTcb());

        hal::ContextSwitch(ScheduleAndUpdateThreads(true, ThreadState::kSleeping));
    }
}

void Scheduler::SetupNextTimeEvent_(const u64 time_ns)
{
    ASSERT_TRUE(HardwareModule::Get().GetEventClockRegistry().IsSelectedPicked());

    auto &event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    ASSERT_TRUE(event_clock.flags.IsCoreLocal, "Scheduler supports only core local event clocks");

    event_clock.cbs.next_event(&event_clock, time_ns);
}

bool Scheduler::WakeUpTasks()
{
    bool should_preempt = false;
    while (!sleep_queue_.IsEmpty()) {
        const u64 time       = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
        const u64 sleep_time = sleep_queue_.Min()->key;

        if (sleep_time > time && sleep_time - time > kMinDelta) {
            break;
        }

        const auto thread = sleep_queue_.Min();
        should_preempt |= IsFirstHigherPriority_(thread, hardware::GetCoreLocalTcb());

        sleep_queue_.Delete(thread);
        ASSERT_EQ(thread->state, ThreadState::kSleeping);

        thread->state = ThreadState::kReady;
        AddReadyThread(thread);
    }

    return should_preempt;
}

Thread *Scheduler::TimerRoutine()
{
    if (hardware::GetCoreLocalTcb() == nullptr) {
        // Not yet converted to scheduling
        return nullptr;
    }

    // TODO: Idle
    return ScheduleAndUpdateThreads(false, ThreadState::kReady);
}
}  // namespace Sched
