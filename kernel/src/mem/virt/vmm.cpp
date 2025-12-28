#include "mem/virt/vmm.hpp"

#include <bits_ext.hpp>
#include <macros.hpp>

#include "hal/constants.hpp"
#include "mem/heap.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/types.hpp"
#include "mem/virt/addr_space.hpp"
#include "mem/virt/area.hpp"
#include "modules/memory.hpp"
#include "trace_framework.hpp"

namespace Mem
{

using std::expected;
using std::unexpected;

using Vmm = VirtualMemoryManager;
using AS  = AddressSpace;

TODO_WHEN_MULTITHREADING
void Vmm::Init(hal::Tlb &tlb, hal::Mmu &mmu, BuddyPmm &pmm) noexcept
{
    DEBUG_INFO_MEMORY("VirtualMemoryManager::Init()");
    tlb_  = &tlb;
    mmu_  = &mmu;
    bpmm_ = &pmm;
}

expected<VirtualPtr<AddressSpace>, MemError> Vmm::CreateAddrSpace()
{
    auto as_res = KNew<AddressSpace>(nullptr);
    RET_UNEXPECTED_IF_ERR(as_res);
    return *as_res;
}

expected<void, MemError> Vmm::DestroyAddrSpace(VPtr<AddressSpace> as)
{
    KDelete(as);
    return {};
}

void Vmm::SwitchAddrSpace(VPtr<AddressSpace> as)
{
    mmu_->SwitchRoot(as->PageTableRoot());
    // SwitchRoot does CR3 load which flushes TLB
}

expected<VPtr<void>, MemError> Vmm::AddArea(VPtr<AddrSp> as, VMemArea vma)
{
    auto res = as->AddArea(vma);
    RET_UNEXPECTED_IF_ERR(res);

    return vma.start;
}

expected<void, MemError> Vmm::RmArea(VPtr<AddrSp> as, VPtr<void> region_start)
{
    auto a_or_err = as->FindArea(region_start);
    RET_UNEXPECTED_IF_ERR(a_or_err);
    auto *area  = *a_or_err;
    auto *start = area->start;
    auto size   = area->size;

    auto &pmt = MemoryModule::Get().GetPageMetaTable();
    hal::KernelMmuContext ctx{*bpmm_, pmt};

    auto unmap_res = mmu_->UnmapRange(ctx, as->PageTableRoot(), start, size);
    RET_UNEXPECTED_IF_ERR(unmap_res);

    auto err = as->RmArea(region_start);
    RET_UNEXPECTED_IF_ERR(err);

    // TLB invalidation handled inside UnMapRange or via explicit flush if needed.
    // But UnMapRange implementation in HAL Mmu calls Unmap which calls invlpg.

    return {};
}

expected<void, MemError> Vmm::UpdateAreaFlags(
    VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
)
{
    auto a_or_err = as->FindArea(region_start);
    RET_UNEXPECTED_IF_ERR(a_or_err);
    auto *area = *a_or_err;
    RET_UNEXPECTED_IF(area->start != region_start, MemError::InvalidArgument);

    area->flags = vmaf;

    bool is_kernel = (PtrToUptr(region_start) >= hal::kKernelVirtualAddressStart);

    hal::PageFlags pf{
        .Present        = true,
        .Writable       = vmaf.writable,
        .UserAccessible = !is_kernel,
        .WriteThrough   = false,
        .CacheDisable   = false,
        .Global         = is_kernel,
        .NoExecute      = !vmaf.executable
    };

    uptr start = PtrToUptr(area->start);
    uptr end   = start + area->size;

    for (uptr v = start; v < end; v += hal::kPageSizeBytes) {
        auto res = mmu_->SetPageFlags(as->PageTableRoot(), UptrToPtr<void>(v), pf);
        RET_UNEXPECTED_IF(!res && res.error() != MemError::NotFound, res.error());
    }

    return {};
}

}  // namespace Mem
