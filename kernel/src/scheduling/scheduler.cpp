#include "scheduling/scheduler.hpp"

#include "autogen/feature_flags.h"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"
#include "scheduling/local_lock.hpp"

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
    policies_[static_cast<size_t>(SchedulingPolicy::kNormalTasks_MLFQ_P3)] =
        PreparePolicy<MLFQPolicy>(&policy3_);
    policies_[static_cast<size_t>(SchedulingPolicy::kBackgroundTasks_RR_P4)] =
        PreparePolicy<RoundRobinPolicy>(&policy4_);
}

void Scheduler::BlockOnWaitQueue(WaitQueue<Thread, kWaitQueueIntrusiveLevel> *wq)
{
    ASSERT_EQ(hardware::GetCoreLocalTcb()->state, ThreadState::kRunning);
    ASSERT_NOT_NULL(wq);

    LocalCoreLock core_lock{};
    wq->EnqueueLast(hardware::GetCoreLocalTcb());
    OnThreadYield_(hardware::GetCoreLocalTcb());
    hal::ContextSwitch(ScheduleAndUpdateThreads(true, ThreadState::kBlockedOnWaitQueue));
}

void Scheduler::ReleaseAndProcessAllBeforeProcessing(
    WaitQueue<Thread, kWaitQueueIntrusiveLevel> *wq
)
{
    ASSERT_EQ(hardware::GetCoreLocalTcb()->state, ThreadState::kRunning);
    ASSERT_NOT_NULL(wq);

    LocalCoreLock core_lock{};
    while (!wq->IsEmpty()) {
        /* Wake up all waiting processes before proceeding */

        auto thread = wq->Dequeue();
        hal::ContextSwitch(ScheduleAndUpdateThreads(false, ThreadState::kReady, thread));
    }
}

void Scheduler::RemoveThread(Thread *thread)
{
    ASSERT_NOT_NULL(thread);
    ASSERT(
        thread->state == ThreadState::kSleeping || thread->state == ThreadState::kReady ||
        thread->state == ThreadState::kBlockedOnWaitQueue
    );

    if (thread->state == ThreadState::kSleeping) {
        ASSERT_TRUE(sleep_queue_.Contains(thread));
        sleep_queue_.Delete(thread);
    } else if (thread->state == ThreadState::kReady) {
        RemoveFromPolicy_(thread);
    } else if (thread->state == ThreadState::kBlockedOnWaitQueue) {
        using wq = WaitQueue<Thread, kWaitQueueIntrusiveLevel>;
        wq::Remove(thread);
    }
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
    ASSERT_NOT_NULL(thread);
    ASSERT_EQ(thread->state, ThreadState::kReady);

    LocalCoreLock lock{};

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

    return nullptr;
}

Thread *Scheduler::ScheduleAndUpdateThreads(
    const bool preempt, const ThreadState thread_state, Thread *forced_next_thread
)
{
    u64 min_time_ns = std::numeric_limits<u64>::max();

    // 1. Wake up all tasks
    const bool force_preempt = WakeUpTasks();

    // 2. Check for sleepers time
    const u64 time = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    if (!sleep_queue_.IsEmpty()) {
        min_time_ns = sleep_queue_.Min()->HookT::key - time;
    }

    // 3. Scheduling new thread if needed and update structs
    Thread *thread{};
    u64 preempt_time_ns = GetPreemptTime_(hardware::GetCoreLocalTcb());
    if (forced_next_thread) {
        /* Forced picked next thread */

        forced_next_thread->state = ThreadState::kReady;
        preempt_time_ns           = GetPreemptTime_(forced_next_thread);
        forced_next_thread->state = ThreadState::kRunning;

        ASSERT_NOT_NULL(hardware::GetCoreLocalTcb());
        ASSERT_EQ(hardware::GetCoreLocalTcb()->state, ThreadState::kRunning);
        hardware::GetCoreLocalTcb()->state = thread_state;

        if (thread_state == ThreadState::kReady) {
            AddReadyThread(hardware::GetCoreLocalTcb());
        }

    } else if (preempt || force_preempt || ShouldPreempt_(preempt_time_ns)) {
        auto next_thread = Schedule();

        if (!next_thread && thread_state == ThreadState::kReady) {
            // Prevent preemption as we are only one running thread...
            hardware::GetCoreLocalTcb()->state = ThreadState::kReady;
            preempt_time_ns                    = GetPreemptTime_(hardware::GetCoreLocalTcb());
            hardware::GetCoreLocalTcb()->state = ThreadState::kRunning;
        } else {
            if (!next_thread) {
                // We are blocking current thread so we have no threads to scheduler -> idle
                next_thread = Idle();
            }

            ASSERT_NOT_NULL(next_thread);

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
    LocalCoreLock lock{};

    // Notify policy that thread is voluntarily yielding
    OnThreadYield_(hardware::GetCoreLocalTcb());

    const auto thread = ScheduleAndUpdateThreads(true, ThreadState::kReady);
    if (thread == nullptr) {
        return;
    }

    hal::ContextSwitch(thread);
}

void Scheduler::ExitThreadUnguarded(const ThreadState state)
{
    hal::ContextSwitch(ScheduleAndUpdateThreads(true, state));
}

void Scheduler::ConvertToScheduling()
{
    TRACE_INFO_SCHEDULING("Converting to scheduling!");

    LocalCoreLock lock{};

    ASSERT_TRUE(HardwareModule::Get().GetEventClockRegistry().IsSelectedPicked());

    auto &event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    ASSERT_TRUE(event_clock.flags.IsCoreLocal, "Scheduler supports only core local event clocks");
    event_clock.cbs.set_oneshot(&event_clock);

    const auto thread = Schedule();
    ASSERT_NOT_NULL(thread);
    TODO_WHEN_MULTICORE

    ASSERT_EQ(thread->state, ThreadState::kReady);

    const u64 preempt_time_ns = GetPreemptTime_(thread);
    ASSERT_GT(preempt_time_ns, kMinDelta);

    thread->state = ThreadState::kRunning;
    SetupNextTimeEvent_(preempt_time_ns);

    TRACE_INFO_SCHEDULING("Converting to thread: %llu!", thread->tid);
    hal::ConvertContext(thread);
}

Thread *Scheduler::Idle() { R_FAIL_ALWAYS("IDLE NOT IMPLEMENTED!"); }

void Scheduler::NanoSleepUntil(const u64 systime_ns)
{
    u64 time;
    {
        LocalCoreLock lock{};
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
        LocalCoreLock lock{};

        hardware::GetCoreLocalTcb()->HookT::key = systime_ns;
        sleep_queue_.Insert(hardware::GetCoreLocalTcb());

        // Notify policy that thread is going to sleep
        OnThreadYield_(hardware::GetCoreLocalTcb());

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
        const u64 sleep_time = sleep_queue_.Min()->HookT::key;

        if (sleep_time > time && sleep_time - time > 2 * kMinDelta) {
            break;
        }

        const auto thread = sleep_queue_.Min();
        ASSERT_NOT_NULL(thread);

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

    // Notify all policies of periodic update
    OnPeriodicUpdate_(hardware::GetCoreLocalTcb());

    // TODO: Idle
    return ScheduleAndUpdateThreads(false, ThreadState::kReady);
}
}  // namespace Sched
