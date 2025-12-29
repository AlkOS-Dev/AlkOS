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

class BuddyPmm;
class Heap;

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
    void Init(
        hal::Tlb &tlb, hal::Mmu &mmu, KernelMmuContext &ctx, Heap &heap,
        const PPtr<void> kernel_root
    ) noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    AddressSpace &GetKernelAddressSpace() { return kernel_as_; }

    expected<VPtr<AddressSpace>, MemError> CreateUserAddrSpace();
    expected<void, MemError> DestroyUserAddrSpace(VPtr<AddressSpace> as);
    void SwitchAddrSpace(VPtr<AddressSpace> as);

    expected<VPtr<void>, MemError> AddArea(VPtr<AddressSpace> as, VMemArea vma);
    expected<void, MemError> RmArea(VPtr<AddressSpace> as, VPtr<void> region_start);
    expected<void, MemError> UpdateAreaFlags(
        VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
    );

    private:
    // ------------------------------
    // Class fields
    // ------------------------------
    hal::Tlb *tlb_;
    hal::Mmu *mmu_;
    KernelMmuContext *ctx_;
    Heap *heap_;
    AddressSpace kernel_as_;
};
using Vmm = VirtualMemoryManager;

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_VMM_HPP_
