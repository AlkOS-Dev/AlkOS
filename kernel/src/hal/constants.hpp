#ifndef KERNEL_SRC_HAL_CONSTANTS_HPP_
#define KERNEL_SRC_HAL_CONSTANTS_HPP_

#include <bits_ext.hpp>
#include <hal/impl/constants.hpp>
#include <types.hpp>

namespace hal
{
static constexpr u64 kKernelVirtualAddressStart = arch::kKernelVirtualAddressStart;

static constexpr u64 kDirectMapAddrStart = arch::kDirectMapAddrStart;
static constexpr u64 kDirectMemMapSizeGb = arch::kDirectMemMapSizeGb;

static constexpr size_t kCacheLineSizeBytes = arch::kCacheLineSizeBytes;
static constexpr size_t kPageSizeBytes      = arch::kPageSizeBytes;

static constexpr u32 kMaxCores = arch::kMaxCores;
static_assert(kMaxCores <= kBitMask16);  // Must fit in u16

using arch::HardwareClockId;
using arch::HardwareEventClockId;
}  // namespace hal

#endif  // KERNEL_SRC_HAL_CONSTANTS_HPP_
