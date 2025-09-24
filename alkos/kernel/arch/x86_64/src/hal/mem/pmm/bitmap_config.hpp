#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BITMAP_CONFIG_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BITMAP_CONFIG_HPP_

#include <extensions/types.hpp>
#include <mem/phys_ptr.hpp>

namespace arch
{

class BitmapPmm;

template <class T>
struct Config;

template <>
struct Config<BitmapPmm> {
    PhysicalPtr<void> pmm_bitmap_addr;
    u64 pmm_total_pages;
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_BITMAP_CONFIG_HPP_
