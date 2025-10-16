#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_AREA_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_AREA_HPP_

#include <extensions/types.hpp>
#include "mem/virt/ptr.hpp"

namespace mem
{

struct MemoryAreaFlags {
    bool readable : 1;
    bool writable : 1;
    bool executable : 1;
    // Future flags: bool user_accessible, bool copy_on_write, etc.
};

struct MemoryArea {
    VirtualPtr<void> start;
    size_t size;
    MemoryAreaFlags flags;

    // Pointer to the next region in the address space for simple linked-list management.
    MemoryArea* next = nullptr;
};

}  // namespace mem

#endif /* ALKOS_KERNEL_INCLUDE_MEM_VIRT_AREA_HPP_ */
