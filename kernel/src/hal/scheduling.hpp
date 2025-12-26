#ifndef KERNEL_SRC_HAL_SCHEDULING_HPP_
#define KERNEL_SRC_HAL_SCHEDULING_HPP_

#include "defines.hpp"
#include "scheduling/thread.hpp"

#include "hal/impl/scheduling.hpp"

namespace hal
{
WRAP_CALL void SwitchToKernelTask(Sched::Thread *thread) { arch::SwitchToKernelTask(thread); }
WRAP_CALL void SwitchToUserTask(Sched::Thread *thread) { arch::SwitchToUserTask(thread); }
WRAP_CALL void ConvertToKernelTask(Sched::Thread *thread) { arch::ConvertToKernelTask(thread); }
WRAP_CALL void InitializeStack(void **stack, void *func) { arch::InitializeStack(stack, func); }
}  // namespace hal

#endif  // KERNEL_SRC_HAL_SCHEDULING_HPP_
