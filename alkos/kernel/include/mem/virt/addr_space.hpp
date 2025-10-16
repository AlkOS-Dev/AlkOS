#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_

#include <extensions/types.hpp>

#include "mem/phys/ptr.hpp"
#include "mem/virt/area.hpp"

namespace mem
{

class AddressSpace
{
    public:
    AddressSpace(PhysicalPtr<void> page_table_root) : page_table_root_{page_table_root} {}
    ~AddressSpace();

    AddressSpace(const AddressSpace&)            = delete;
    AddressSpace& operator=(const AddressSpace&) = delete;

    PhysicalPtr<void> PageTableRoot() const { return page_table_root_; }

    private:
    PhysicalPtr<void> page_table_root_;
    MemoryArea* area_list_head_;
};

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_
