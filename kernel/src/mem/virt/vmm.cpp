#include "mem/virt/vmm.hpp"

#include <bits_ext.hpp>
#include <macros.hpp>
#include <template/scope_guard.hpp>

#include "constants.hpp"
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

    current_as_ = &kernel_as_;
}

expected<VPtr<AddressSpace>, MemError> Vmm::CreateUserAddrSpace()
{
    auto as_res = KNew<AddressSpace>();
    RET_UNEXPECTED_IF_ERR(as_res);
    auto as = *as_res;

    template_lib::ScopeGuard as_guard([&] {
        DestroyUserAddrSpace(as);
    });

    auto init_res = as->InitUser(*ctx_, *mmu_);
    RET_UNEXPECTED_IF_ERR(init_res);

    mmu_->CopyKernelSpace(as->PageTableRoot(), kernel_as_.PageTableRoot());

    // This enables lazy synchronization of kernel mappings into user address spaces.
    auto kernel_sync_vma = Mem::KNew<Mem::KernelSyncVMemArea>();
    RET_UNEXPECTED_IF(!kernel_sync_vma, MemError::OutOfMemory);

    // AddArea takes ownership of the VMA pointer
    auto res = as->AddArea(*kernel_sync_vma);
    RET_UNEXPECTED_IF_ERR(res);

    as_guard.dismiss();
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
    current_as_ = as;
    mmu_->SwitchRoot(as->PageTableRoot());
    // SwitchRoot does CR3 load which flushes TLB
}

expected<VPtr<void>, MemError> Vmm::AddArea(VPtr<AddrSp> as, VMemArea *vma)
{
    auto start = vma->GetStart();
    auto res   = as->AddArea(vma);
    RET_UNEXPECTED_IF_ERR(res);

    return start;
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

expected<VPtr<void>, MemError> Vmm::AllocAnonymous(
    VPtr<AddressSpace> as, size_t size, VirtualMemAreaFlags flags, VPtr<void> range_start,
    VPtr<void> range_end
)
{
    auto gap_res = as->FindGap(size, range_start, range_end);
    RET_UNEXPECTED_IF_ERR(gap_res);
    auto gap = *gap_res;

    auto vma_res = KNew<AnonymousVMemArea>(gap.start, gap.size, flags);
    RET_UNEXPECTED_IF(!vma_res, MemError::OutOfMemory);
    auto *vma = *vma_res;

    UpdateAreaFlags(as, vma->GetStart(), flags);

    auto add_res = as->AddArea(vma);
    RET_UNEXPECTED_IF_ERR(add_res);

    return gap.start;
}

expected<VPtr<void>, MemError> Vmm::AllocUserStack(VPtr<AddressSpace> as, size_t size)
{
    auto user_start = UptrToPtr<void>(kUserSpaceStart);
    auto user_end   = UptrToPtr<void>(kUserSpaceEndExclusive);

    VirtualMemAreaFlags flags = {
        .readable = true, .writable = true, .executable = false, .cache_disable = true
    };

    auto res = AllocAnonymous(as, size, flags, user_start, user_end);
    RET_UNEXPECTED_IF_ERR(res);

    VPtr<void> base = *res;

    if constexpr (hal::kStackGrowsDown) {
        return reinterpret_cast<VPtr<void>>(reinterpret_cast<uptr>(base) + size);
    } else {
        return base;
    }
}

expected<VPtr<void>, MemError> Vmm::AllocKernelHeap(size_t size)
{
    auto kernel_start = UptrToPtr<void>(kKernelSpaceStart);

    VirtualMemAreaFlags flags = {
        .readable = true, .writable = true, .executable = false, .cache_disable = false
    };

    return AllocAnonymous(&kernel_as_, size, flags, kernel_start, nullptr);
}

}  // namespace Mem
