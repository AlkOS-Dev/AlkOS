#ifndef ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_TPP_
#define ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_TPP_

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
        exception_table_[irq].logical_irq = irq;
    }

    for (u16 irq = 0; irq < kNumHardwareExceptions; irq++) {
        hardware_exception_table_[irq].logical_irq = irq;
    }

    for (u16 irq = 0; irq < software_exception_table_; irq++) {
        software_exception_table_[irq].logical_irq = irq;
    }
}
}  // namespace intr

#endif  // ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_TPP_
