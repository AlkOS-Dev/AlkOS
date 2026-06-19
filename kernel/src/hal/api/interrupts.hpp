// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_API_INTERRUPTS_HPP_
#define KERNEL_SRC_HAL_API_INTERRUPTS_HPP_

#include "stddef.h"

namespace arch
{
class Interrupts;

struct InterruptsAPI {
    /* Should perform full initialisation of interrupt system */
    void Init();

    /* Safety methods */
    void BlockHardwareInterrupts();
    void EnableHardwareInterrupts();
};

}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_INTERRUPTS_HPP_
