// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_INTERRUPTS_HPP_
#define KERNEL_SRC_HAL_INTERRUPTS_HPP_

#include <hal/impl/interrupts.hpp>

namespace hal
{
using arch::Interrupts;
static constexpr size_t kMaxInterruptsSupported = arch::kMaxInterruptsSupported;
}  // namespace hal

#endif  // KERNEL_SRC_HAL_INTERRUPTS_HPP_
