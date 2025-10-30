#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_LOCAL_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_LOCAL_HPP_

#include "hal/constants.hpp"
#include "hal/core.hpp"

namespace hardware
{
struct alignas(hal::kCacheLineSizeBytes) CoreLocal {
    u8 nested_interrupts{};
    u16 lid{};
};

FAST_CALL CoreLocal &GetCoreLocalData()
{
    return *static_cast<CoreLocal *>(hal::GetCoreLocalData());
}
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_LOCAL_HPP_
