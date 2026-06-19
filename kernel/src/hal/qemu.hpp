// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_QEMU_HPP_
#define KERNEL_SRC_HAL_QEMU_HPP_

#include <hal/impl/qemu.hpp>

namespace hal
{
/* This is plain bad. But it's also quick to implement and easy to delete
 * when time comes for proper abstraction */
WRAP_CALL void QemuShutdown() { arch::QemuShutdown(); }
}  // namespace hal

#endif  // KERNEL_SRC_HAL_QEMU_HPP_
