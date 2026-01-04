#ifndef KERNEL_SRC_SCHEDULING_LOCAL_LOCK_HPP_
#define KERNEL_SRC_SCHEDULING_LOCAL_LOCK_HPP_

#include <defines.hpp>

#include "hal/sync.hpp"
#include "modules/hardware.hpp"

class LocalCoreLock
{
    hal::CpuInterruptFlags state_{};

    public:
    FORCE_INLINE_F LocalCoreLock()
    {
        state_ = hal::GetCpuInterruptFlags();
        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    }
    ~LocalCoreLock() { hal::RestoreCpuInterruptFlags(state_); }
};

#endif  // KERNEL_SRC_SCHEDULING_LOCAL_LOCK_HPP_
