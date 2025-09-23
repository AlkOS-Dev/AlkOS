#ifndef ALKOS_KERNEL_INCLUDE_MEM_VMM_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VMM_HPP_

// TODO: This include is here because of conflicing paths
// This requires solving
#include <hal/mem/vmm.hpp>

namespace mem
{

class VirtualMemoryManager final : public arch::VirtualMemoryManager
{
};
using Vmm = VirtualMemoryManager;

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VMM_HPP_
