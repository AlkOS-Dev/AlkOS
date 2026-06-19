// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HARDWARE_INTERRUPT_DRIVER_HPP_
#define KERNEL_SRC_HARDWARE_INTERRUPT_DRIVER_HPP_

#include <types.h>

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
