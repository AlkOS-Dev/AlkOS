#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_HPP_

#include <mem/vmm_abi.hpp>
#include "hal/mem/pmm/buddy.hpp"
#include "hal/mem/vmm/impl.hpp"

namespace arch
{

class VirtualMemoryManager : public VirtualMemoryManagerImpl<BuddyPmm>
{
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_HPP_
