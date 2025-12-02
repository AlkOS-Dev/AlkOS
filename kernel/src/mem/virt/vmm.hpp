#ifndef KERNEL_SRC_MEM_VIRT_VMM_HPP_
#define KERNEL_SRC_MEM_VIRT_VMM_HPP_

#include <expected.hpp>
#include <types.hpp>

#include "hal/mmu.hpp"
#include "hal/tlb.hpp"
#include "mem/error.hpp"
#include "mem/types.hpp"
#include "mem/virt/addr_space.hpp"

namespace Mem
{

using std::expected;
using std::unexpected;

//==============================================================================
// VMM
//==============================================================================

class VirtualMemoryManager
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    VirtualMemoryManager() = default;
    void Init(hal::Tlb &tlb, hal::Mmu &mmu) noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    expected<VPtr<AddressSpace>, MemError> CreateAddrSpace();
    expected<void, MemError> DestroyAddrSpace(VPtr<AddressSpace> as);
    void SwitchAddrSpace(VPtr<AddressSpace> as);

    expected<VPtr<void>, MemError> AddArea(VPtr<AddressSpace> as, VMemArea vma);
    expected<void, MemError> RmArea(VPtr<AddressSpace> as, VPtr<void> region_start);
    // expected<void, MemError> UpdateAreaFlags(
    //     VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
    // );

    private:
    // ------------------------------
    // Class fields
    // ------------------------------
    hal::Tlb *tlb_;
    hal::Mmu *mmu_;
};
using Vmm = VirtualMemoryManager;

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_VMM_HPP_
