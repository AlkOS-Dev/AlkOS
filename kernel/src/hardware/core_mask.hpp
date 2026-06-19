// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HARDWARE_CORE_MASK_HPP_
#define KERNEL_SRC_HARDWARE_CORE_MASK_HPP_

#include "data_structures/bit_array.hpp"
#include "hal/constants.hpp"

namespace hardware
{
using CoreMask = data_structures::BitArray<hal::kMaxCores>;
}  // namespace hardware

#endif  // KERNEL_SRC_HARDWARE_CORE_MASK_HPP_
