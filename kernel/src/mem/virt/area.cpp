#include "mem/virt/area.hpp"

#include <string.h>
#include <bits_ext.hpp>
#include <hal/constants.hpp>
#include <hal/panic.hpp>
#include <mem/mmu/contexts.hpp>
#include <mem/virt/addr_space.hpp>
#include <modules/memory.hpp>
#include <trace_framework.hpp>

namespace Mem
{

static bool MapPage(AddressSpace &as, VPtr<void> vaddr, PPtr<void> paddr, VirtualMemAreaFlags flags)
{
    auto &mmu      = MemoryModule::Get().GetMmu();
    bool is_kernel = (Mem::PtrToUptr(vaddr) >= hal::kKernelVirtualAddressStart);

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

}  // namespace Mem
