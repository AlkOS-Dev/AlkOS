#include "mem/virt/area.hpp"

#include <string.h>
#include <bits_ext.hpp>

#include "constants.hpp"
#include "hal/constants.hpp"
#include "hal/panic.hpp"
#include "mem/mmu/contexts.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/memory.hpp"
#include "trace_framework.hpp"

namespace Mem
{

static bool MapPage(AddressSpace &as, VPtr<void> vaddr, PPtr<void> paddr, VirtualMemAreaFlags flags)
{
    auto &mmu      = MemoryModule::Get().GetMmu();
    bool is_kernel = (Mem::PtrToUptr(vaddr) >= kKernelSpaceStart);

    hal::PageFlags page_flags{
        .Present        = true,
        .Writable       = flags.writable,
        .UserAccessible = !is_kernel,
        .WriteThrough   = flags.write_through,
        .CacheDisable   = flags.cache_disable,
        .Global         = is_kernel,
        .NoExecute      = !flags.executable
    };

    // We use the kernel context for mapping operations
    auto &mmu_ctx = MemoryModule::Get().GetKernelMmuContext();

    auto res = mmu.Map(mmu_ctx, as.PageTableRoot(), vaddr, paddr, page_flags);

    if (!res) {
        TRACE_WARN_MEMORY(
            "Failed to map page at %p -> %p: %s", vaddr, paddr, to_string(res.error())
        );
        return false;
    }

    return true;
}

static bool CheckWritePermissions(
    VPtr<void> addr, const PageFaultData::ErrorCode &err, VirtualMemAreaFlags flags
)
{
    if (err.write && !flags.writable) {
        hal::KernelPanicFormat("Write to read-only memory area at %p", addr);
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// AnonymousVMemArea
// -----------------------------------------------------------------------------

bool AnonymousVMemArea::HandleFault(
    VPtr<void> fault_addr, const PageFaultData::ErrorCode &err, AddressSpace &as
)
{
    if (!CheckWritePermissions(fault_addr, err, flags_)) {
        return false;
    }

    TRACE_INFO_MEMORY("Handling Anonymous Fault at %p", fault_addr);

    auto &pmm = MemoryModule::Get().GetBitmapPmm();

    auto page_res = pmm.Alloc();
    if (!page_res) {
        TRACE_FATAL_MEMORY("OOM during anonymous page fault");
        return false;
    }

    PPtr<void> phys_page = *page_res;

    // TODO: optimization - zero page optimization (CoW zero page)
    VPtr<void> virt_page = Mem::PhysToVirt(phys_page);
    memset(virt_page, 0, hal::kPageSizeBytes);

    VPtr<void> aligned_vaddr = AlignDown(fault_addr, hal::kPageSizeBytes);
    return MapPage(as, aligned_vaddr, phys_page, flags_);
}

// -----------------------------------------------------------------------------
// DirectMappingVMemArea
// -----------------------------------------------------------------------------

bool DirectMappingVMemArea::HandleFault(
    VPtr<void> fault_addr, const PageFaultData::ErrorCode &err, AddressSpace &as
)
{
    if (!CheckWritePermissions(fault_addr, err, flags_)) {
        return false;
    }

    TRACE_INFO_MEMORY("Handling Direct Mapping Fault at %p", fault_addr);

    // Calculate physical address based on offset
    u64 offset         = Mem::PtrToUptr(fault_addr) - Mem::PtrToUptr(start_);
    uptr phys_addr_val = Mem::PtrToUptr(phys_start_) + offset;

    PPtr<void> phys_page     = Mem::UptrToPtr<void>(AlignDown(phys_addr_val, hal::kPageSizeBytes));
    VPtr<void> aligned_vaddr = AlignDown(fault_addr, hal::kPageSizeBytes);

    return MapPage(as, aligned_vaddr, phys_page, flags_);
}

// -----------------------------------------------------------------------------
// KernelSyncVMemArea
// -----------------------------------------------------------------------------

KernelSyncVMemArea::KernelSyncVMemArea()
    // Covers the entire upper half of the 64-bit address space
    // 0x8000000000000000 - 0xFFFFFFFFFFFFFFFF
    : VMemArea(
          Mem::UptrToPtr<void>(kKernelSpaceStart), kKernelSpaceEndInclusive - kKernelSpaceStart,
          // Permissions are determined by the actual kernel page table entries being copied,
          // these flags are just placeholders for the VMA object.
          VirtualMemAreaFlags{.readable = true, .writable = true, .executable = true}
      )
{
}

bool KernelSyncVMemArea::HandleFault(
    VPtr<void> fault_addr, const PageFaultData::ErrorCode &, AddressSpace &as
)
{
    auto &vmm = MemoryModule::Get().GetVmm();

    auto kernel_root  = vmm.GetKernelAddressSpace().PageTableRoot();
    auto current_root = as.PageTableRoot();

    if (current_root == kernel_root) {
        TRACE_FATAL_MEMORY("Kernel Page Fault in Kernel Address Space at %p", fault_addr);
        return false;
    }

    auto &mmu = MemoryModule::Get().GetMmu();
    if (mmu.SyncMapping(current_root, kernel_root, fault_addr)) {
        MemoryModule::Get().GetTlb().InvalidatePage(AlignDown(fault_addr, hal::kPageSizeBytes));
        return true;
    }

    TRACE_FATAL_MEMORY("Kernel Sync Fault failed at %p (not mapped in kernel)", fault_addr);
    return false;
}

}  // namespace Mem
