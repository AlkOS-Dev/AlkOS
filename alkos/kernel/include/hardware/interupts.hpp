#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_

#include <hal/interrupts.hpp>
#include <hal/spinlock.hpp>

namespace hardware
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
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    Interrupts() noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F void HandleInterrupt(const u16 idx)
    {
        ASSERT_LT(idx, hal::kMaxInterruptsSupported);

        if (InterruptHandlerEntry& entry = handler_table_[idx]; entry.handler_data.handler) {
            (*entry.handler_data.handler)(entry);
        }
    }

    FORCE_INLINE_F void InstallInterruptHandler(
        const u16 idx, const InterruptHandlerEntry::HandlerData& handler
    )
    {
        ASSERT_LT(idx, hal::kMaxInterruptsSupported);
        handler_table_[idx].handler_data = handler;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    InterruptHandlerEntry handler_table_[hal::kMaxInterruptsSupported];
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
