#ifndef ALKOS_KERNEL_INCLUDE_MEM_VMM_VMM_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VMM_VMM_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/phys_ptr.hpp"
#include "mem/virt_ptr.hpp"
#include "mem/vmm/addr_space.hpp"

namespace mem
{

//==============================================================================
// Aliases
//==============================================================================

template <typename T, typename E>
using Expected = std::expected<T, E>;

//==============================================================================
// VMM
//==============================================================================

class VirtualMemoryManager
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit VirtualMemoryManager(AddressSpace as) noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    Expected<VirtualPtr<AddressSpace>, MemError> CreateAddrSpace();
    Expected<void, MemError> DestroyAddrSpace();
    void SwitchAddrSpace(AddressSpace as);

    // Returns handle to it so it can be deleted?
    using MemRegHandle = u64;
    Expected<MemRegHandle, MemError> AddRegion(MemoryRegion mr);
    Expected<void, MemError> RmRegion(MemRegHandle mrh);
    Expected<void, MemError> UpdateRegionFlags(MemoryRegion mrh, MemoryRegionFlags mrf);

    void Map(VirtualPtr<void> vaddr, PhysicalPtr<void> paddr);
    void UnMap(VirtualPtr<void> vaddr, PhysicalPtr<void> paddr);

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    AddressSpace current_as_;
};

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VMM_VMM_HPP_
