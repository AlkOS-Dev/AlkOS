#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_

#include <hal/interrupts.hpp>

namespace hardware
{
class Interrupts final : public arch::Interrupts
{
    // ------------------------------
    // Class defines
    // ------------------------------

    public:
    struct InterruptHandlerEntry {
        using InterruptHandler = void (*)(u16 idx, InterruptHandlerEntry& entry);
        InterruptHandler handler{};
        void* data{};
    };

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F void HandleInterrupt(const u16 idx)
    {
        ASSERT_LT(idx, hal::kMaxInterruptsSupported);

        if (InterruptHandlerEntry& entry = handler_table_[idx]; entry.handler) {
            (*entry.handler)(idx, entry);
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    InterruptHandlerEntry handler_table_[hal::kMaxInterruptsSupported];
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
