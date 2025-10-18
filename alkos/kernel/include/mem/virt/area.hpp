#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_AREA_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_AREA_HPP_

#include <extensions/types.hpp>
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

namespace Mem
{

TODO_STD_VARIANT
enum class VirtualMemAreaT { Anonymous, DirectMapping };

struct VirtualMemAreaFlags {
    bool readable : 1;
    bool writable : 1;
    bool executable : 1;
    // Future flags: bool user_accessible, bool copy_on_write, etc.
};
using VMemAreaFlags = VirtualMemAreaFlags;

struct VirtualMemArea {
    VirtualPtr<void> start;
    size_t size;
    VirtualMemAreaFlags flags;

    TODO_STD_VARIANT
    VirtualMemAreaT type;
    PhysicalPtr<void> direct_mapping_start;

    VirtualMemArea* next = nullptr;
};
using VMemArea = VirtualMemArea;

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_AREA_HPP_
