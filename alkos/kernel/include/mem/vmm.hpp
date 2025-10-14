#ifndef ALKOS_KERNEL_INCLUDE_MEM_VMM_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VMM_HPP_

#include <hal/vmm.hpp>

namespace mem
{

class VirtualMemoryManager final : public arch::VirtualMemoryManager
{
};
using Vmm = VirtualMemoryManager;

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VMM_HPP_
