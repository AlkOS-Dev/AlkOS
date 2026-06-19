// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_BOOT_ARGS_HPP_
#define KERNEL_SRC_HAL_BOOT_ARGS_HPP_

#include <hal/impl/kernel.hpp>

namespace hal
{
using arch::RawBootArguments;

WRAP_CALL void ArchInit(const RawBootArguments &args) { arch::ArchInit(args); }
}  // namespace hal

#endif  // KERNEL_SRC_HAL_BOOT_ARGS_HPP_
