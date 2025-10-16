#ifndef ALKOS_KERNEL_INCLUDE_MEM_VMM_VMM_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VMM_VMM_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/virt/addr_space.hpp"
#include "mem/virt/ptr.hpp"
#include "mem/phys/ptr.hpp"

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
    Expected<void, MemError> DestroyAddrSpace(VirtualPtr<AddressSpace> as);
    void SwitchAddrSpace(VirtualPtr<AddressSpace> as);

    // Returns handle to it so it can be deleted?
    using MemoryAreaHandle = u64;
    Expected<MemoryAreaHandle, MemError> AddRegion(VirtualPtr<AddressSpace> as, MemoryArea ma);
    Expected<void, MemError> RmRegion(VirtualPtr<AddressSpace> as, MemoryAreaHandle mah);
    Expected<void, MemError> UpdateRegionFlags(VirtualPtr<AddressSpace> as, MemoryArea ma, MemoryAreaFlags maf);

    private:
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
