#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_MASK_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_MASK_HPP_

#include "extensions/data_structures/bit_array.hpp"
#include "hal/constants.hpp"

namespace hardware
{
using CoreMask = data_structures::BitArray<hal::kMaxCores>;
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_MASK_HPP_
