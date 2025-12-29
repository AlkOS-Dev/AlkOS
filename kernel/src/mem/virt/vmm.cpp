#include "mem/virt/vmm.hpp"

#include <bits_ext.hpp>
#include <macros.hpp>

#include "hal/constants.hpp"
#include "mem/heap.hpp"
#include "mem/mmu/contexts.hpp"
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
void Vmm::Init(
    hal::Tlb &tlb, hal::Mmu &mmu, KernelMmuContext &ctx, Heap &heap, const PPtr<void> kernel_root
) noexcept
{
    DEBUG_INFO_MEMORY("VirtualMemoryManager::Init()");
    tlb_ = &tlb;
    mmu_ = &mmu;
    ctx_ = &ctx;
    // Heap is used implicitly via KNew / KDelete
    heap_ = &heap;
    (void)heap;

    TRACE_INFO_MEMORY("Initializing Kernel Address Space");
    auto init_res = kernel_as_.InitKernel(kernel_root, *ctx_, *mmu_);
    R_ASSERT_TRUE(init_res);
}

expected<VPtr<AddressSpace>, MemError> Vmm::CreateUserAddrSpace()
{
    auto as_res = KNew<AddressSpace>();
    RET_UNEXPECTED_IF_ERR(as_res);
    auto as = *as_res;

    auto init_res = as->InitUser(*ctx_, *mmu_);
    RET_UNEXPECTED_IF_ERR(init_res);

    return as;
}

expected<void, MemError> Vmm::DestroyUserAddrSpace(VPtr<AddressSpace> as)
{
    mmu_->ClearUserMappings(*ctx_, as->PageTableRoot());
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
    auto hint_res = as->RmArea(region_start);
    RET_UNEXPECTED_IF_ERR(hint_res);
    auto hint = *hint_res;

    tlb_->InvalidateRange(hint.start, hint.size);

    return {};
}

expected<void, MemError> Vmm::UpdateAreaFlags(
    VPtr<AddressSpace> as, VPtr<void> region_start, VirtualMemAreaFlags vmaf
)
{
    auto hint_res = as->UpdateAreaFlags(region_start, vmaf);
    RET_UNEXPECTED_IF_ERR(hint_res);
    auto hint = *hint_res;

    tlb_->InvalidateRange(hint.start, hint.size);

    return {};
}

}  // namespace Mem
