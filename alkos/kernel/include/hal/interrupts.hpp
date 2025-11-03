#ifndef ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_

#include <hal/impl/interrupts.hpp>

namespace hal
{
using arch::Interrupts;
static constexpr size_t kMaxInterruptsSupported = arch::kMaxInterruptsSupported;
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_
