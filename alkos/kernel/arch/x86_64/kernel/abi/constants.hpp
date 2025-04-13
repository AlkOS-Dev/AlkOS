#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CONSTANTS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CONSTANTS_HPP_

#include <extensions/bit.hpp>

namespace arch
{
static constexpr u64 kKernelVirtualAddressStart   = BitMaskLeft<u64, 33>;
static constexpr u64 kKernelDirectMapAddressStart = BitMaskLeft<u64, 17>;
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CONSTANTS_HPP_
