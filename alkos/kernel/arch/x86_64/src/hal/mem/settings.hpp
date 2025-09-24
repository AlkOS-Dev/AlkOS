#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_SETTINGS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_SETTINGS_HPP_

#include "hal/mem/pmm/bitmap.hpp"
#include "hal/mem/vmm/impl.hpp"

namespace arch
{

namespace internal
{

using PmmImpl   = BitmapPmm;
using PmmConfig = BitmapPmmConfig;

using VmmImpl   = VirtualMemoryManager<PmmImpl>;
using VmmConfig = VirtualMemoryManagerConfig;

}  // namespace internal

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_SETTINGS_HPP_
