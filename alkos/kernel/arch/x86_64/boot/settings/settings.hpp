#ifndef ALKOS_BOOT_SETTINGS_HPP_
#define ALKOS_BOOT_SETTINGS_HPP_

#include <extensions/types.hpp>
#include <extensions/bit.hpp>

static constexpr u64 kKernelVirtualAddressStart   = kBitMaskLeft<u64, 33>;
static constexpr u64 kKernelDirectMapAddressStart = kBitMaskLeft<u64, 17>;

#endif  // ALKOS_BOOT_SETTINGS_HPP_
