#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_

#include "scheduling/thread.hpp"

extern "C" void ConvertToKernelTask(Sched::Thread *thread);
extern "C" void SwitchToKernelTask(Sched::Thread *thread);
extern "C" void SwitchToUserTask(Sched::Thread *thread);

namespace arch
{
using ::ConvertToKernelTask;
using ::SwitchToKernelTask;
using ::SwitchToUserTask;
void InitializeStack(void **stack, void (*f)());
}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_
