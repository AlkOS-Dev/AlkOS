#ifndef ALKOS_BOOT_LIB_MEM_PMM_HPP_
#define ALKOS_BOOT_LIB_MEM_PMM_HPP_

#include "multiboot2/memory_map.hpp"

class PhysicalMemoryManager
{
    PhysicalMemoryManager(Multiboot::MemoryMap memory_map);
}

#endif  // ALKOS_BOOT_LIB_MEM_PMM_HPP_
