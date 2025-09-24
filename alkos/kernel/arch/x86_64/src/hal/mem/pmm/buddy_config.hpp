#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BUDDY_CONFIG_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BUDDY_CONFIG_HPP_

#include <extensions/utility.hpp>

#include "hal/mem/pmm/bitmap_config.hpp"
#include "hal/mem/vmm/impl_config.hpp"

namespace arch
{

class BuddyPmm;

template <class T>
struct Config;

template <>
struct Config<BuddyPmm> {
    Config<BitmapPmm> bitmap_pmm_config;
    Config<VirtualMemoryManager> vmm_config;
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BUDDY_CONFIG_HPP_
