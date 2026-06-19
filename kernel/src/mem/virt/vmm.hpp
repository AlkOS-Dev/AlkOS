// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MEM_VIRT_VMM_HPP_
#define KERNEL_SRC_MEM_VIRT_VMM_HPP_

#include <types.h>
#include <expected.hpp>

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
    AddressSpace &GetCurrentAddressSpace() { return *current_as_; }

    expected<VPtr<AddressSpace>, MemError> CreateUserAddrSpace();
    expected<void, MemError> DestroyUserAddrSpace(VPtr<AddressSpace> as);
    void SwitchAddrSpace(VPtr<AddressSpace> as);

    expected<VPtr<void>, MemError> AddArea(VPtr<AddressSpace> as, VMemArea *vma);
    expected<void, MemError> RmArea(VPtr<AddressSpace> as, VPtr<void> region_start);
    expected<void, MemError> UpdateAreaFlags(
        VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
    );

    // ------------------------------
    // Allocation Helpers
    // ------------------------------

    expected<VPtr<void>, MemError> AllocAnonymous(
        VPtr<AddressSpace> as, size_t size, VirtualMemAreaFlags flags,
        VPtr<void> range_start = nullptr, VPtr<void> range_end = nullptr
    );

    expected<VPtr<void>, MemError> AllocUserStack(VPtr<AddressSpace> as, size_t size);
    expected<VPtr<void>, MemError> AllocUserHeap(VPtr<AddressSpace> as, size_t size);

    expected<VPtr<void>, MemError> AllocKernelHeap(size_t size);

    expected<VPtr<void>, MemError> MapUserBackbuffer(
        VPtr<AddressSpace> as, PPtr<void> buffer, size_t size_bytes
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
    AddressSpace *current_as_;
};
using Vmm = VirtualMemoryManager;

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_VMM_HPP_
