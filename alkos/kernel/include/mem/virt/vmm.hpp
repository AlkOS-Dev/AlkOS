#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_VMM_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_VMM_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/types.hpp"

namespace Mem
{

//==============================================================================
// Aliases
//==============================================================================

template <typename T, typename E>
using Expected = std::expected<T, E>;
template <typename E>
using Unexpected = std::unexpected<E>;

//==============================================================================
// VMM
//==============================================================================

class VirtualMemoryManager
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit VirtualMemoryManager() noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    Expected<VPtr<AddressSpace>, MemError> CreateAddrSpace();
    Expected<void, MemError> DestroyAddrSpace(VPtr<AddressSpace> as);
    void SwitchAddrSpace(VPtr<AddressSpace> as);

    Expected<VPtr<void>, MemError> AddArea(VPtr<AddressSpace> as, VirtualMemArea vma);
    Expected<void, MemError> RmArea(VPtr<AddressSpace> as, VPtr<void> region_start);
    // Expected<void, MemError> UpdateAreaFlags(
    //     VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
    // );

    private:
    // ------------------------------
    // Class fields
    // ------------------------------
};

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_VMM_HPP_
