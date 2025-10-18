#ifndef ALKOS_KERNEL_INCLUDE_HAL_INTERRUPT_PARAMS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_INTERRUPT_PARAMS_HPP_

#include <hal/impl/interrupt_params.hpp>

namespace hal
{
using arch::ExceptionData;
static constexpr size_t kNumExceptionHandlers  = arch::kNumExceptionHandlers;
static constexpr size_t kNumHardwareInterrupts = arch::kNumHardwareInterrupts;
static constexpr size_t kNumSoftwareInterrupts = arch::kNumSoftwareInterrupts;
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_INTERRUPT_PARAMS_HPP_
