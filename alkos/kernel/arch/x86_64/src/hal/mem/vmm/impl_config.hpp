#ifndef ALKOS_KERNEL_ARCH_x86_64_SRC_HAL_MEM_VMM_IMPL_CONFIG_HPP_
#define ALKOS_KERNEL_ARCH_x86_64_SRC_HAL_MEM_VMM_IMPL_CONFIG_HPP_

#include <extensions/template_lib.hpp>
#include <mem/phys_ptr.hpp>

#include "mem/page_map.hpp"

namespace arch
{

class VirtualMemoryManager;

template <class T>
struct Config;

template <>
struct Config<VirtualMemoryManager> {
    PhysicalPtr<PageMapTable<4>> pml4_table;
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_x86_64_SRC_HAL_MEM_VMM_IMPL_CONFIG_HPP_
