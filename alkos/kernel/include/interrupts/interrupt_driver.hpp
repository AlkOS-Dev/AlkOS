#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_INTERRUPT_DRIVER_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_INTERRUPT_DRIVER_HPP_

#include <extensions/types.hpp>

namespace intr
{
struct interrupt_driver {
    struct callbacks {
        void (*ack)(interrupt_driver&);
    };

    u16 logical_irq{};
    u64 hardware_irq{};
    callbacks* cbs{};
};
}  // namespace intr

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_INTERRUPT_DRIVER_HPP_
