#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_

#include <hal/api/pmm.hpp>

#include "mem/settings.hpp"

namespace arch
{

class PhysicalMemoryManager : public internal::PmmImpl
{
};

using PmmConfig = internal::PmmConfig;

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_
