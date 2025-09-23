#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_TPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_TPP_

#include "hal/mem/vmm/impl.hpp"

#include <extensions/template_lib.hpp>
#include <mem/phys_ptr.hpp>
#include <mem/virt_ptr.hpp>

#include "hal/mem/vmm/impl_config.hpp"
#include "mem/page_map.hpp"

namespace arch
{

template <class PmmT>
    requires std::is_base_of_v<PhysicalMemoryManagerABI, PmmT>
void VirtualMemoryManagerImpl<PmmT>::InitImpl()
{
    const auto &c = this->GetConfig();
    pm_table_4_   = c.pml4_table;
}

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_TPP_
