#ifndef ALKOS_KERNEL_ABI_CONSTANTS_HPP_
#define ALKOS_KERNEL_ABI_CONSTANTS_HPP_

#include <extensions/data_structures/array_structures.hpp>

using DeviceName = data_structures::StringArray<8>;
static_assert(sizeof(DeviceName) == sizeof(u64));

/**
 * Expected fields in arch namespace:
 * - u64 kKernelVirtualAddressStart;
 * - u64 kKernelDirectMapAddressStart;
 * - size_t kCacheLineSizeBytes;
 */

namespace arch
{
/**
 * This enum should define all possible hardware clock IDs.
 * The values should encode name in 8bytes, and never equal 0.
 */
enum class HardwareClockId : u64;
}  // namespace arch

#include <abi/constants.hpp>

#endif  // ALKOS_KERNEL_ABI_CONSTANTS_HPP_
