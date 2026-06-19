// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_QEMU_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_QEMU_HPP_

#include <defines.hpp>
#include "cpu/utils.hpp"
#include "include/io.hpp"
#include "todo.hpp"

// TODO
namespace arch
{
WRAP_CALL void QemuShutdown()
{
    outw(0x604, 0x2000);
    OsHang();
}
}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_QEMU_HPP_
