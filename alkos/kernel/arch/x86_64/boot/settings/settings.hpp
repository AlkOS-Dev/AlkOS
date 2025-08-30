#ifndef ALKOS_BOOT_SETTINGS_HPP_
#define ALKOS_BOOT_SETTINGS_HPP_

#include <extensions/bit.hpp>
#include <extensions/types.hpp>

static constexpr u64 kKernelVirtualAddressStart   = kBitMaskLeft<u64, 33>;
static constexpr u64 kKernelDirectMapAddressStart = kBitMaskLeft<u64, 17>;

#endif  // ALKOS_BOOT_SETTINGS_HPP_
