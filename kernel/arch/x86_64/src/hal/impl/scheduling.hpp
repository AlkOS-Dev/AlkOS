#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_

#include "scheduling/thread.hpp"

extern "C" void SwitchToTask(Sched::Thread *thread);

namespace arch
{
using ::SwitchToTask;
}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SCHEDULING_HPP_
