#ifndef KERNEL_SRC_HARDWARE_INTERRUPT_DRIVER_HPP_
#define KERNEL_SRC_HARDWARE_INTERRUPT_DRIVER_HPP_

#include <types.hpp>

namespace intr
{
struct interrupt_driver {
    struct callbacks {
        void (*ack)(interrupt_driver &);
    };

    u16 logical_irq{};
    u64 hardware_irq{};
    callbacks *cbs{};
};
}  // namespace intr

#endif  // KERNEL_SRC_HARDWARE_INTERRUPT_DRIVER_HPP_
