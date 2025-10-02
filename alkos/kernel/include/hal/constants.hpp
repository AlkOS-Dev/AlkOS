#ifndef ALKOS_KERNEL_HAL_CONSTANTS_HPP_
#define ALKOS_KERNEL_HAL_CONSTANTS_HPP_

/**
 * Expected fields in arch namespace:
 * - u64 kKernelVirtualAddressStart;
 * - u64 kDirectMapAddrStart;
 */

#include <types.h>

#include <hal/impl/constants.hpp>

static constexpr u64 kKernelVirtualAddressStart = arch::kKernelVirtualAddressStart;

static constexpr u64 kDirectMapAddrStart = arch::kDirectMapAddrStart;
static constexpr u64 kDirectMemMapSizeGb = arch::kDirectMemMapSizeGb;

static constexpr size_t kCacheLineSizeBytes = arch::kCacheLineSizeBytes;
static constexpr size_t kPageSizeBytes      = arch::kPageSizeBytes;

using HardwareClockId = arch::HardwareClockId;

#endif  // ALKOS_KERNEL_HAL_CONSTANTS_HPP_
