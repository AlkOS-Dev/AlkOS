#ifndef KERNEL_SRC_HAL_SCHEDULING_HPP_
#define KERNEL_SRC_HAL_SCHEDULING_HPP_

#include "defines.hpp"
#include "scheduling/thread.hpp"

#include "hal/impl/scheduling.hpp"

namespace hal
{
WRAP_CALL void JumpToUserSpace(void (*f)()) { arch::JumpToUserSpace(f); }
WRAP_CALL void ContextSwitch(Sched::Thread *thread) { arch::ContextSwitch(thread); }
WRAP_CALL void ConvertContext(Sched::Thread *thread) { arch::ConvertContext(thread); }
WRAP_CALL void InitializeThreadStack(void **stack, Sched::Task task)
{
    arch::InitializeThreadStack(stack, task);
}
}  // namespace hal

#endif  // KERNEL_SRC_HAL_SCHEDULING_HPP_
