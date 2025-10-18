#ifndef ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_HPP_
#define ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_HPP_

#include <extensions/cstddef.hpp>

namespace intr
{
template <
    std::size_t kNumExceptions, std::size_t kNumHardwareExceptions,
    std::size_t kNumSoftwareExceptions>
class LogicalInterruptTable
{
    // struct InterruptHandlerEntry {
    //     struct HandlerData {
    //         using InterruptHandler = void (*)(InterruptHandlerEntry& entry);
    //         InterruptHandler handler{};
    //         void* data{};
    //     };
    //
    //     HandlerData handler_data{};
    //     u16 irq{};
    //     hal::Spinlock spinlock{};
    //     interrupt_driver driver{};
    // };
    // FORCE_INLINE_F void HandleInterrupt(const u16 lirq)
    // {
    //     ASSERT_LT(lirq, hal::kMaxInterruptsSupported);
    //
    //     InterruptHandlerEntry& entry = handler_table_[lirq];
    //     ASSERT_NOT_NULL(entry.driver.cbs, "Interrupt driver is not installed!");
    //
    //     if (entry.handler_data.handler) {
    //         (*entry.handler_data.handler)(entry);
    //     }
    //
    //     entry.driver.cbs->ack(entry.driver);
    // }
    //
    // FORCE_INLINE_F void InstallInterruptHandler(
    //     const u16 lirq, const InterruptHandlerEntry::HandlerData& handler
    // )
    // {
    //     ASSERT_LT(lirq, hal::kMaxInterruptsSupported);
    //     handler_table_[lirq].handler_data = handler;
    // }
    //
    // FORCE_INLINE_F void MapLogicalInterruptToHw(const u16 lirq, const u64 hardware_irq)
    // {
    //     ASSERT_LT(lirq, hal::kMaxInterruptsSupported);
    //     handler_table_[lirq].driver.hardware_irq = hardware_irq;
    // }
    //
    // FORCE_INLINE_F void InstallInterruptDriver(const u16 lirq, interrupt_driver::callbacks*
    // driver)
    // {
    //     ASSERT_LT(lirq, hal::kMaxInterruptsSupported);
    //     handler_table_[lirq].driver.cbs = driver;
    // }

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    LogicalInterruptTable();

    // ------------------------------
    // Class interaction
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
};
}  // namespace intr

#include "logical_interrupt_table.tpp"

#endif  // ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_HPP_
