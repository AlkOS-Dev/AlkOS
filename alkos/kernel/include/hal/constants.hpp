#ifndef ALKOS_KERNEL_INCLUDE_HAL_CONSTANTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_CONSTANTS_HPP_

#include <types.h>
#include <extensions/bits_ext.hpp>
#include <extensions/cstddef.hpp>
#include <hal/impl/constants.hpp>

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

#endif  // ALKOS_KERNEL_INCLUDE_HAL_CONSTANTS_HPP_
