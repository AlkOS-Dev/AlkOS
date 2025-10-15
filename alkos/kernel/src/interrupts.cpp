#include "hardware/interupts.hpp"

namespace hardware
{
Interrupts::Interrupts() noexcept
{
    for (u16 irq = 0; irq < hal::kMaxInterruptsSupported; irq++) {
        handler_table_[irq].irq = irq;
    }
}
}  // namespace hardware
