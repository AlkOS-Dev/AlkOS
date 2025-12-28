#include "scheduling/scheduler.hpp"

#include "hal/scheduling.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"

void Sched::Scheduler::AddReadyThread(Thread *thread)
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

void Sched::Scheduler::Schedule()
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    auto thread = GetNext_();

    auto owner = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(owner));

    if (owner.value()->flags.KernelSpaceOnly) {
        hal::SwitchToKernelTask(thread);
    } else {
        hal::SwitchToUserTask(thread);
    }
}

void Sched::Scheduler::ConvertToScheduling()
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    auto thread = GetNext_();

    auto owner = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(owner));

    if (owner.value()->flags.KernelSpaceOnly) {
        hal::ConvertToKernelTask(thread);
    } else {
        R_FAIL_ALWAYS("Not implemented...");
    }
}

Sched::Thread *Sched::Scheduler::GetNext_()
{
    Thread *thread = threads_;
    threads_       = threads_->next;
    return thread;
}
