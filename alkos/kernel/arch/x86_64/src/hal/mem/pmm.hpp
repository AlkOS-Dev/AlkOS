#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_

#include <mem/pmm_abi.hpp>
#include "hal/mem/pmm/buddy.hpp"

namespace arch
{

class Pmm : public BuddyPmm
{
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_PMM_HPP_
