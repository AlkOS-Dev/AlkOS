#ifndef ALKOS_KERNEL_INCLUDE_HAL_INTERRUPT_PARAMS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_INTERRUPT_PARAMS_HPP_

#include <hal/impl/interrupt_params.hpp>

namespace hal
{
/* Interrupt layout specific params */
using arch::ExceptionData;
static constexpr size_t kNumExceptionHandlers  = arch::kNumExceptionHandlers;
static constexpr size_t kNumHardwareInterrupts = arch::kNumHardwareInterrupts;
static constexpr size_t kNumSoftwareInterrupts = arch::kNumSoftwareInterrupts;

/* Mapping params */
static constexpr u16 kTimerHwLirq      = 0;
static constexpr u16 kPageFaultExcLirq = 14;
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_INTERRUPT_PARAMS_HPP_
