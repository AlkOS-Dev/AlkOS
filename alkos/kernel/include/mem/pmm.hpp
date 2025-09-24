#ifndef ALKOS_KERNEL_INCLUDE_MEM_PMM_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PMM_HPP_

#include <hal/mem/pmm.hpp>

namespace mem
{

class PhysicalMemoryManager final : public arch::PhysicalMemoryManager
{
};
using Pmm = PhysicalMemoryManager;

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PMM_HPP_
