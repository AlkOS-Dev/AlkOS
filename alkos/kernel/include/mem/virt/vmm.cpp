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
    // Later invalidate TLB etc
    KFree(as);
    return {};
}

void Vmm::SwitchAddrSpace(VPtr<AddressSpace> as)
{
    mmu_.SwitchAddrSpace(as);
    tlb_.FlushAll();
}

Expected<VPtr<void>, MemError> Vmm::AddArea(VPtr<AddrSp> as, VMemArea vma)
{
    auto res = as->AddArea(vma);
    EXPECTED_RET_IF_ERR(res);
}

Expected<void, MemError> Vmm::RmArea(VPtr<AddrSp> as, VPtr<void> region_start)
{
    auto res = as->RmArea(region_start);
    EXPECTED_RET_IF_ERR(res);
}

}  // namespace Mem
