#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_

#include <defines.hpp>

#include "scheduling/thread.hpp"

extern "C" void ConvertContext(Sched::Thread *thread);
extern "C" void ContextSwitch(Sched::Thread *thread);
extern "C" void JumpToUserSpace(void (*f)());

namespace arch
{
using ::ContextSwitch;
using ::ConvertContext;
using ::JumpToUserSpace;
void InitializeThreadStack(void **stack, Sched::Task task);
}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_
