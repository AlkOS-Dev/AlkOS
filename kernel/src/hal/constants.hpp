// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_CONSTANTS_HPP_
#define KERNEL_SRC_HAL_CONSTANTS_HPP_

#include <types.h>
#include <bits_ext.hpp>
#include <hal/impl/constants.hpp>

namespace hal
{

static constexpr u64 kDirectMapAddrStart = arch::kDirectMapAddrStart;
static constexpr u64 kDirectMemMapSizeGb = arch::kDirectMemMapSizeGb;

static constexpr size_t kCacheLineSizeBytes = arch::kCacheLineSizeBytes;
static constexpr size_t kPageSizeBytes      = arch::kPageSizeBytes;
static constexpr size_t kPageShift          = arch::kPageShift;

static constexpr u32 kMaxCores = arch::kMaxCores;
static_assert(kMaxCores <= kBitMask16);  // Must fit in u16

static constexpr u16 kElfMachineType = arch::kElfMachineType;

static constexpr bool kStackGrowsDown = arch::kStackGrowsDown;

using arch::HardwareClockId;
using arch::HardwareEventClockId;

}  // namespace hal

#endif  // KERNEL_SRC_HAL_CONSTANTS_HPP_
