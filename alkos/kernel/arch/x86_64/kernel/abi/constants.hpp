#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CONSTANTS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CONSTANTS_HPP_

#include <extensions/bit.hpp>
#include <extensions/types.hpp>
#include <todo.hpp>

namespace arch
{

// ------------------------------
// Constants
// ------------------------------

static constexpr u64 kKernelVirtualAddressStart   = kBitMaskLeft<u64, 33>;
static constexpr u64 kKernelDirectMapAddressStart = kBitMaskLeft<u64, 17>;
static constexpr size_t kCacheLineSizeBytes       = 64;

// ------------------------------
// Enums
// ------------------------------

enum class HardwareClockId : u64 {
    kTsc                     = data_structures::StringArrayToIntegral(DeviceName("TSC")),
    kHpet                    = data_structures::StringArrayToIntegral(DeviceName("HPET")),
    kPit                     = data_structures::StringArrayToIntegral(DeviceName("PIT")),
    TODO_DEVICE_SUPPORT kRtc = data_structures::StringArrayToIntegral(DeviceName("RTC")),
    TODO_DEVICE_SUPPORT kInterruptBased =
        data_structures::StringArrayToIntegral(DeviceName("INTRCLCK")),
    TODO_DEVICE_SUPPORT
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CONSTANTS_HPP_
