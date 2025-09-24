#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BUDDY_CONFIG_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BUDDY_CONFIG_HPP_

#include <extensions/utility.hpp>

#include "hal/mem/pmm/bitmap_config.hpp"
#include "hal/mem/vmm/impl_config.hpp"

namespace arch
{

struct BuddyPmmConfig {
    BitmapPmmConfig bitmap_pmm_config;
    VirtualMemoryManagerConfig vmm_config;

    BuddyPmmConfig(BitmapPmmConfig bitmap_config, VirtualMemoryManagerConfig vmm_config)
        : bitmap_pmm_config(std::move(bitmap_config)), vmm_config(std::move(vmm_config))
    {
    }
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BUDDY_CONFIG_HPP_
