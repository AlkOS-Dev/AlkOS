#ifndef KERNEL_SRC_HAL_SCHEDULING_HPP_
#define KERNEL_SRC_HAL_SCHEDULING_HPP_

#include "defines.hpp"
#include "scheduling/thread.hpp"

#include "hal/impl/scheduling.hpp"

namespace hal
{
WRAP_CALL void SwitchToTask(Sched::Thread *thread) { arch::SwitchToTask(thread); }
}  // namespace hal

#endif  // KERNEL_SRC_HAL_SCHEDULING_HPP_
