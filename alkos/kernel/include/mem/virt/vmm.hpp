#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_VMM_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_VMM_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "hal/mmu.hpp"
#include "hal/tlb.hpp"
#include "mem/error.hpp"
#include "mem/types.hpp"
#include "mem/virt/addr_space.hpp"

namespace Mem
{

//==============================================================================
// VMM
//==============================================================================

class VirtualMemoryManager
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit VirtualMemoryManager(hal::Tlb &tlb, hal::Mmu &mmu) noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    Expected<VPtr<AddressSpace>, MemError> CreateAddrSpace();
    Expected<void, MemError> DestroyAddrSpace(VPtr<AddressSpace> as);
    void SwitchAddrSpace(VPtr<AddressSpace> as);

    Expected<VPtr<void>, MemError> AddArea(VPtr<AddressSpace> as, VMemArea vma);
    Expected<void, MemError> RmArea(VPtr<AddressSpace> as, VPtr<void> region_start);
    // Expected<void, MemError> UpdateAreaFlags(
    //     VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
    // );

    private:
    // ------------------------------
    // Class fields
    // ------------------------------
    hal::Tlb &tlb_;
    hal::Mmu &mmu_;
};
using Vmm = VirtualMemoryManager;

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_VMM_HPP_
