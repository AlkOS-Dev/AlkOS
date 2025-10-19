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

/* Mapping params - each architecture MUST define this */
static constexpr u16 kTimerHwLirq      = arch::kTimerHwLirq;
static constexpr u16 kPageFaultExcLirq = arch::kPageFaultExcLirq;
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_INTERRUPT_PARAMS_HPP_
