// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_INTERRUPT_PARAMS_HPP_
#define KERNEL_SRC_HAL_INTERRUPT_PARAMS_HPP_

#include <defines.hpp>
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

NODISCARD FAST_CALL bool IsInterruptFromUserSpace(const ExceptionData &data)
{
    return arch::IsInterruptFromUserSpace(data);
}

}  // namespace hal

#endif  // KERNEL_SRC_HAL_INTERRUPT_PARAMS_HPP_
