#ifndef ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_

#include <hal/impl/interrupts.hpp>

namespace hal
{
using arch::ExceptionData;
using arch::Interrupts;
static constexpr size_t kNumExceptionHandlers  = arch::kNumExceptionHandlers;
static constexpr size_t kNumHardwareInterrupts = arch::kNumHardwareInterrupts;
static constexpr size_t kNumSoftwareInterrupts = arch::kNumSoftwareInterrupts;
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_
