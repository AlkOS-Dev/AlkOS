#ifndef ALKOS_BOOT_LIB_SYS_CONSTANTS_HPP_
#define ALKOS_BOOT_LIB_SYS_CONSTANTS_HPP_

#include <extensions/bit.hpp>
#include <extensions/types.hpp>

namespace arch
{
static constexpr u64 kKernelVirtualAddressStart   = kBitMaskLeft<u64, 33>;
static constexpr u64 kKernelDirectMapAddressStart = kBitMaskLeft<u64, 17>;

static constexpr size_t kCacheLineSizeBytes = 64;
}  // namespace arch

#endif  // ALKOS_BOOT_LIB_SYS_CONSTANTS_HPP_
