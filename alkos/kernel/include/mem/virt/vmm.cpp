#include <extensions/debug.hpp>

#include "mem/heap.hpp"
#include "mem/types.hpp"
#include "mem/virt/addr_space.hpp"
#include "mem/virt/area.hpp"
#include "mem/virt/vmm.hpp"

namespace Mem
{

using Vmm = VirtualMemoryManager;
using AS  = AddressSpace;

TODO_WHEN_MULTITHREADING
Vmm::VirtualMemoryManager(hal::Tlb &tlb, hal::Mmu &mmu) noexcept : tlb_{tlb}, mmu_{mmu}
{
    TRACE_INFO("VirtualMemoryManager::VirtualMemoryManager()");
}

Expected<VirtualPtr<AddressSpace>, MemError> Vmm::CreateAddrSpace()
{
    return KMalloc<AddressSpace>();
}

Expected<void, MemError> Vmm::DestroyAddrSpace(VPtr<AddressSpace> as)
{
    for (const VMemArea &vma : *as) {
        // Unmap all pages in the VMA
        mmu_.UnMapRange(as, vma.start, vma.size);
        tlb_.InvalidateRange(vma.start, vma.size);
    }
    KFree(as);
    return {};
}

void Vmm::SwitchAddrSpace(VPtr<AddressSpace> as)
{
    mmu_.SwitchRootPageMapTable(as->PageTableRoot());
    tlb_.FlushAll();
}

Expected<VPtr<void>, MemError> Vmm::AddArea(VPtr<AddrSp> as, VMemArea vma)
{
    auto res = as->AddArea(vma);
    UNEXPETED_RET_IF_ERR(res);

    return vma.start;
}

Expected<void, MemError> Vmm::RmArea(VPtr<AddrSp> as, VPtr<void> region_start)
{
    // Get area start/end
    auto a_or_err = as->FindArea(region_start);
    UNEXPETED_RET_IF_ERR(a_or_err);
    auto area  = *a_or_err;
    auto start = area->start;
    auto size  = area->size;

    // Unmap the range
    auto unmap_res = mmu_.UnMapRange(as, start, size);
    UNEXPETED_RET_IF_ERR(unmap_res);

    // Remove from address space
    auto err = as->RmArea(region_start);
    UNEXPETED_RET_IF_ERR(err);

    // Invalidate TLB
    tlb_.InvalidateRange(start, size);
    return {};
}

}  // namespace Mem
