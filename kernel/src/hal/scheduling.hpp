#ifndef KERNEL_SRC_HAL_SCHEDULING_HPP_
#define KERNEL_SRC_HAL_SCHEDULING_HPP_

#include "defines.hpp"
#include "scheduling/thread.hpp"

#include "hal/impl/scheduling.hpp"

namespace hal
{
WRAP_CALL void ContextSwitch(Sched::Thread *thread) { arch::ContextSwitch(thread); }
WRAP_CALL void ConvertContext(Sched::Thread *thread) { arch::ConvertContext(thread); }
WRAP_CALL void InitializeStackKThread(void **stack, void (*f)())
{
    arch::InitializeStackKThread(stack, f);
}
WRAP_CALL void InitializeStackUserThread(void **stack, void (*f)())
{
    arch::InitializeStackUserThread(stack, f);
}
}  // namespace hal

#endif  // KERNEL_SRC_HAL_SCHEDULING_HPP_
