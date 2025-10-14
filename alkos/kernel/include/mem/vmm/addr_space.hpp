#ifndef ALKOS_KERNEL_INCLUDE_MEM_VMM_ADDR_SPACE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VMM_ADDR_SPACE_HPP_

#include "extensions/types.hpp"

namespace mem
{

struct AddressSpace {
    size_t start;
    size_t end;
    uptr page_table_phys_addr;
};

struct MemoryRegionFlags {
    bool read : 1;
    bool write : 1;
};

struct MemoryRegion {
    uptr start;
    size_t size;
    MemoryRegionFlags flags;
};

}  // namespace mem

#endif /* ALKOS_KERNEL_INCLUDE_MEM_VMM_ADDR_SPACE_HPP_ */
