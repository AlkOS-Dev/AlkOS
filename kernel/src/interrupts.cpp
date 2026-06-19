// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "hardware/interupts.hpp"

namespace intr
{
Interrupts::Interrupts() noexcept
{
    for (u16 irq = 0; irq < hal::kMaxInterruptsSupported; irq++) {
        handler_table_[irq].irq                = irq;
        handler_table_[irq].driver.logical_irq = irq;
    }
}
}  // namespace intr
