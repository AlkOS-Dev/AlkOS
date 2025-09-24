#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_TPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_TPP_

#include "hal/mem/vmm/impl.hpp"

#include <extensions/template_lib.hpp>
#include <mem/phys_ptr.hpp>
#include <mem/virt_ptr.hpp>

#include "hal/mem/vmm/impl_config.hpp"
#include "mem/page_map.hpp"

namespace arch::internal
{

template <class PmmT>
void VirtualMemoryManager<PmmT>::InitImpl()
{
    const auto &c = this->GetConfig();
    pm_table_4_   = c.pml4_table;
}

// TODO : Implement this, and implement the ABI in sensible way

}  // namespace arch::internal

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_TPP_
