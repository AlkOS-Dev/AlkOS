#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_CONSTANTS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_CONSTANTS_HPP_

#include <extensions/bit.hpp>
#include <extensions/types.hpp>

namespace arch
{
static constexpr u64 kKernelVirtualAddressStart = kBitMaskLeft<u64, 33>;
static constexpr u64 kDirectMapAddrStart        = kBitMaskLeft<u64, 17>;
static constexpr u64 kDirectMemMapSizeGb        = 512;

static constexpr size_t kCacheLineSizeBytes = 64;
static constexpr size_t kPageSizeBytes      = 4096;
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_CONSTANTS_HPP_
