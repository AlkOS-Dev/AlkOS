#ifndef KERNEL_SRC_SCHEDULING_SCHEDULER_LOCK_HPP_
#define KERNEL_SRC_SCHEDULING_SCHEDULER_LOCK_HPP_

#include <defines.hpp>

#include "modules/hardware.hpp"

class LocalCoreLock
{
    public:
    FORCE_INLINE_F LocalCoreLock()
    {
        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    }
    ~LocalCoreLock() { HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts(); }
};

#endif  // KERNEL_SRC_SCHEDULING_SCHEDULER_LOCK_HPP_
