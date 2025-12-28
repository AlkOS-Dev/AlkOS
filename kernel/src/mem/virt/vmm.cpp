#include "mem/virt/vmm.hpp"
#include <macros.hpp>
#include "mem/heap.hpp"
#include "mem/types.hpp"
#include "mem/virt/addr_space.hpp"
#include "mem/virt/area.hpp"
#include "trace_framework.hpp"

namespace Mem
{

using std::expected;
using std::unexpected;

using Vmm = VirtualMemoryManager;
using AS  = AddressSpace;

TODO_WHEN_MULTITHREADING
void Vmm::Init(hal::Tlb &tlb, hal::Mmu &mmu) noexcept
{
    DEBUG_INFO_MEMORY("VirtualMemoryManager::Init()");
    tlb_ = &tlb;
    mmu_ = &mmu;
}

expected<VirtualPtr<AddressSpace>, MemError> Vmm::CreateAddrSpace()
{
    return KMalloc<AddressSpace>();
}

expected<void, MemError> Vmm::DestroyAddrSpace(VPtr<AddressSpace> as)
{
    for (const VMemArea &vma : *as) {
        auto res = RmArea(as, vma.start);
        UNEXPECTED_RET_IF_ERR(res);
    }
    KFree(as);
    return {};
}

void Vmm::SwitchAddrSpace(VPtr<AddressSpace> as)
{
    mmu_->SwitchRootPageMapTable(as->PageTableRoot());
    tlb_->FlushAll();
}

expected<VPtr<void>, MemError> Vmm::AddArea(VPtr<AddrSp> as, VMemArea vma)
{
    auto res = as->AddArea(vma);
    UNEXPECTED_RET_IF_ERR(res);

    return vma.start;
}

expected<void, MemError> Vmm::RmArea(VPtr<AddrSp> as, VPtr<void> region_start)
{
    // Get area start/end
    auto a_or_err = as->FindArea(region_start);
    UNEXPECTED_RET_IF_ERR(a_or_err);
    auto *area  = *a_or_err;
    auto *start = area->start;
    auto size   = area->size;

    // Unmap the range
    auto unmap_res = mmu_->UnMapRange(as, start, size);
    UNEXPECTED_RET_IF_ERR(unmap_res);

    // Remove from address space
    auto err = as->RmArea(region_start);
    UNEXPECTED_RET_IF_ERR(err);

    // Invalidate TLB
    tlb_->InvalidateRange(start, size);
    return {};
}

expected<void, MemError> Vmm::UpdateAreaFlags(
    VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
)
{
    auto a_or_err = as->FindArea(region_start);
    UNEXPECTED_RET_IF_ERR(a_or_err);
    auto *area = *a_or_err;
    if (area->start != region_start) {
        return unexpected(MemError::InvalidArgument);
    }

    area->flags = vmaf;
    hal::PageFlags pf{
        .Present        = true,
        .Writable       = vmaf.writable,
        .UserAccessible = true,  // User space VMA
        .WriteThrough   = false,
        .CacheDisable   = false,
        .Global         = false,
        .NoExecute      = !vmaf.executable
    };

    uptr start = PtrToUptr(area->start);
    uptr end   = start + area->size;

    for (uptr v = start; v < end; v += hal::kPageSizeBytes) {
        auto res = mmu_->SetPageFlags(as, UptrToPtr<void>(v), pf);
        if (res) {
            tlb_->InvalidatePage(UptrToPtr<void>(v));
        } else if (res.error() != MemError::NotFound) {
            return unexpected(res.error());
        }
        // NotFound means page not mapped yet (lazy allocation),
        // which is fine as future faults will use new VMA flags.
    }

    return {};
}

}  // namespace Mem
