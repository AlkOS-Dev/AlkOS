#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_

#include <mem/pmm_abi.hpp>

#include "hal/mem/settings.hpp"

namespace arch
{

class PhysicalMemoryManager : public internal::PmmImpl
{
};

using PmmConfig = internal::PmmConfig;

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_
