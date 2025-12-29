#include "scheduling/scheduler.hpp"

#include "hal/scheduling.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"

namespace Sched
{
void Scheduler::AddReadyThread(Thread *thread)
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();

    if (threads_ == nullptr) {
        threads_     = thread;
        thread->next = thread;
        return;
    }

    thread->next   = threads_->next;
    threads_->next = thread;
    threads_       = thread;

    HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();
}

Thread *Scheduler::Schedule()
{
    ASSERT_NOT_NULL(threads_);

    auto thread = GetNext_();
    ASSERT_NOT_NULL(thread);

    auto owner = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(owner));

    if (thread == hardware::GetCurrentTCB()) {
        return nullptr;
    }

    DEBUG_INFO_SCHEDULING("Schedule() returns thread with tid: %llu", thread->tid);
    return thread;
}
void Scheduler::Yield()
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    auto thread = Schedule();

    if (thread != nullptr) {
        hal::ContextSwitch(thread);
    }
}

void Scheduler::ConvertToScheduling()
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();

    static constexpr u64 kPeriodicTime1Ms = kNanosInSecond / 1'000;
    TimingModule::Get().GetEventFramework().SetupPeriodic(kPeriodicTime1Ms);

    ASSERT_NOT_NULL(threads_);

    auto thread = GetNext_();
    ASSERT_NOT_NULL(thread);

    auto owner = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(owner));

    hal::ConvertContext(thread);
}

Thread *Scheduler::GetNext_()
{
    Thread *thread = threads_;
    threads_       = threads_->next;
    return thread;
}
}  // namespace Sched
