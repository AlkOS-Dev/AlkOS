#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_SETTINGS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_SETTINGS_HPP_

#include "hal/mem/pmm/bitmap.hpp"
#include "hal/mem/vmm/impl.hpp"

namespace arch
{

namespace internal
{
using PmmImpl   = BitmapPmm;
using VmmImpl   = VirtualMemoryManagerImpl<PmmImpl>;
using PmmConfig = BitmapPmmConfig;
using VmmConfig = VirtualMemoryManagerImplConfig;
}  // namespace internal

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_SETTINGS_HPP_
