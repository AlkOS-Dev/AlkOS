#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_

#include <hal/interrupts.hpp>
#include <hal/spinlock.hpp>
#include "hardware/interrupt_driver.hpp"

namespace intr
{
class Interrupts final : public arch::Interrupts
{
    // ------------------------------
    // Class defines
    // ------------------------------

    public:
    struct InterruptHandlerEntry {
        struct HandlerData {
            using InterruptHandler = void (*)(InterruptHandlerEntry& entry);
            InterruptHandler handler{};
            void* data{};
        };

        HandlerData handler_data{};
        u16 irq{};
        hal::Spinlock spinlock{};
        interrupt_driver driver{};
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    Interrupts() noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F void HandleInterrupt(const u16 lirq)
    {
        ASSERT_LT(lirq, hal::kMaxInterruptsSupported);

        InterruptHandlerEntry& entry = handler_table_[lirq];
        ASSERT_NOT_NULL(entry.driver.cbs->ack);

        if (entry.handler_data.handler) {
            (*entry.handler_data.handler)(entry);
        }

        entry.driver.cbs->ack(entry.driver);
    }

    FORCE_INLINE_F void InstallInterruptHandler(
        const u16 lirq, const InterruptHandlerEntry::HandlerData& handler
    )
    {
        ASSERT_LT(lirq, hal::kMaxInterruptsSupported);
        handler_table_[lirq].handler_data = handler;
    }

    FORCE_INLINE_F void MapLogicalInterruptToHw(const u16 lirq, const u64 hardware_irq)
    {
        ASSERT_LT(lirq, hal::kMaxInterruptsSupported);
        handler_table_[lirq].driver.hardware_irq = hardware_irq;
    }

    FORCE_INLINE_F void InstallInterruptDriver(const u16 lirq, interrupt_driver::callbacks* driver)
    {
        ASSERT_LT(lirq, hal::kMaxInterruptsSupported);
        handler_table_[lirq].driver.cbs = driver;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    InterruptHandlerEntry handler_table_[hal::kMaxInterruptsSupported];
};
}  // namespace intr

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
