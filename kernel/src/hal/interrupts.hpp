#ifndef KERNEL_SRC_HAL_INTERRUPTS_HPP_
#define KERNEL_SRC_HAL_INTERRUPTS_HPP_

#include <hal/impl/interrupts.hpp>

namespace hal
{
using arch::Interrupts;
static constexpr size_t kMaxInterruptsSupported = arch::kMaxInterruptsSupported;
}  // namespace hal

#endif  // KERNEL_SRC_HAL_INTERRUPTS_HPP_
