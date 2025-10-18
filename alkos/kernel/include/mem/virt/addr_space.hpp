#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/area.hpp"

namespace Mem
{

//==============================================================================
// Forward Declarations
//==============================================================================

class VirtualMemoryManager;

//==============================================================================
// Aliases
//==============================================================================

template <typename T, typename E>
using Expected = std::expected<T, E>;
template <typename E>
using Unexpected = std::unexpected<E>;

//==============================================================================
// AddressSpace
//==============================================================================

class AddressSpace
{
    public:
    AddressSpace(PPtr<void> page_table_root) : page_table_root_{page_table_root} {}
    ~AddressSpace();
    AddressSpace(const AddressSpace&)            = delete;
    AddressSpace& operator=(const AddressSpace&) = delete;

    PPtr<void> PageTableRoot() const { return page_table_root_; }

    private:
    // This is orchestrated in VMM (For proper TLB management)
    void AddArea(VMemArea vma);
    void RmArea(VPtr<void> ptr);

    // Helpers
    Expected<VPtr<VMemArea>, MemError> FindArea(VPtr<void> ptr);
    bool IsAddrInArea(VPtr<VMemArea> vma, VPtr<void> ptr);

    PPtr<void> page_table_root_;
    VPtr<VMemArea> area_list_head_;

    friend VirtualMemoryManager;
};
using AddrSp = AddressSpace;

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_HPP_
