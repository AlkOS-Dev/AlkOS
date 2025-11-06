#ifndef KERNEL_SRC_HARDWARE_CORE_MASK_HPP_
#define KERNEL_SRC_HARDWARE_CORE_MASK_HPP_

#include "data_structures/bit_array.hpp"
#include "hal/constants.hpp"

namespace hardware
{
using CoreMask = data_structures::BitArray<hal::kMaxCores>;
}  // namespace hardware

#endif  // KERNEL_SRC_HARDWARE_CORE_MASK_HPP_
