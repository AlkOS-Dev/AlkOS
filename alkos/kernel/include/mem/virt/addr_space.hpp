#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/types.hpp"
#include "mem/virt/addr_space_iterator.hpp"
#include "mem/virt/area.hpp"

namespace Mem
{

//==============================================================================
// Forward Declarations
//==============================================================================

class VirtualMemoryManager;

//==============================================================================
// AddressSpace
//==============================================================================

class AddressSpace
{
    public:
    AddressSpace(PPtr<void> page_table_root) : page_table_root_{page_table_root} {}
    ~AddressSpace();
    AddressSpace(const AddressSpace &)            = delete;
    AddressSpace &operator=(const AddressSpace &) = delete;

    PPtr<void> PageTableRoot() const { return page_table_root_; }

    // Iterator
    AddrSpIt begin() const { return AddrSpIt(area_list_head_); }
    AddrSpIt end() const { return AddrSpIt(nullptr); }

    private:
    // This is orchestrated in VMM (For proper TLB management)
    Expected<void, MemError> AddArea(VMemArea vma);
    Expected<void, MemError> RmArea(VPtr<void> ptr);

    // Helpers
    Expected<VPtr<VMemArea>, MemError> FindArea(VPtr<void> ptr);
    bool IsAddrInArea(VPtr<VMemArea> vma, VPtr<void> ptr);
    bool AreasOverlap(VPtr<VMemArea> a, VPtr<VMemArea> b);

    PPtr<void> page_table_root_;
    VPtr<VMemArea> area_list_head_;

    friend VirtualMemoryManager;

    public:
};
using AddrSp = AddressSpace;

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_
