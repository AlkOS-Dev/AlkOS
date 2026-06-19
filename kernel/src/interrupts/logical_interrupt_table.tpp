// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_TPP_
#define ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_TPP_

#include "hal/sync.hpp"
#include "interrupts/logical_interrupt_table.hpp"

namespace intr
{
template <
    std::size_t kNumExceptions, std::size_t kNumHardwareExceptions,
    std::size_t kNumSoftwareExceptions>
LogicalInterruptTable<
    kNumExceptions, kNumHardwareExceptions, kNumSoftwareExceptions>::LogicalInterruptTable()
{
    for (u16 irq = 0; irq < kNumExceptions; irq++) {
        exception_table_[irq].logical_irq  = irq;
        exception_table_[irq].hardware_irq = kUnmappedIrq;
    }

    for (u16 irq = 0; irq < kNumHardwareExceptions; irq++) {
        hardware_interrupt_table_[irq].logical_irq = irq;
        exception_table_[irq].hardware_irq         = kUnmappedIrq;
    }

    for (u16 irq = 0; irq < kNumSoftwareExceptions; irq++) {
        software_interrupt_table_[irq].logical_irq = irq;
        exception_table_[irq].hardware_irq         = kUnmappedIrq;
    }
}
}  // namespace intr

#endif  // ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_TPP_
