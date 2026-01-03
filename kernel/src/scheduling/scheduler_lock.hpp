#ifndef KERNEL_SRC_SCHEDULING_SCHEDULER_LOCK_HPP_
#define KERNEL_SRC_SCHEDULING_SCHEDULER_LOCK_HPP_

#include <defines.hpp>

#include "modules/hardware.hpp"

class SchedulerLock
{
    public:
    FORCE_INLINE_F SchedulerLock()
    {
        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    }
    ~SchedulerLock() { HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts(); }
};

#endif  // KERNEL_SRC_SCHEDULING_SCHEDULER_LOCK_HPP_
