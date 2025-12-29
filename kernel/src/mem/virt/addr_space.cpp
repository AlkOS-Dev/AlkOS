#include "mem/virt/addr_space.hpp"

#include <macros.hpp>
#include <mutex.hpp>
#include "hal/constants.hpp"
#include "hal/mmu.hpp"
#include "mem/error.hpp"
#include "mem/heap.hpp"
#include "mem/mmu/contexts.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/types.hpp"

using namespace Mem;
using AS = AddressSpace;

AS::AddressSpace() : page_table_root_{nullptr}, owns_page_table_root_{false} {}

expected<void, MemError> AS::InitUser(KernelMmuContext &ctx, hal::Mmu &mmu)
{
    ctx_ = &ctx;
    mmu_ = &mmu;

    auto page_table_root_res = ctx_->AllocateTable(0);
    RET_UNEXPECTED_IF_ERR(page_table_root_res);
    page_table_root_      = *page_table_root_res;
    owns_page_table_root_ = true;

    return {};
}

expected<void, MemError> AS::InitKernel(
    const PPtr<void> kernel_root, KernelMmuContext &ctx, hal::Mmu &mmu
)
{
    page_table_root_      = kernel_root;
    mmu_                  = &mmu;
    ctx_                  = &ctx;
    owns_page_table_root_ = false;

    return {};
}
AS::~AddressSpace()
{
    // Free all allocated areas
    // area_list_ destructor will handle freeing nodes and data

    if (ctx_) {
        if (owns_page_table_root_ && page_table_root_ != nullptr) {
            ctx_->FreeTable(page_table_root_, 0);
        }
    }
}

expected<void, MemError> AS::AddArea(VMemArea vma)
{
    std::lock_guard guard(area_list_lock_);

    // Check for overlapping areas
    for (auto it = area_list_.begin(); it != area_list_.end(); ++it) {
        if (AreasOverlap(&(*it), &vma)) {
            return unexpected(MemError::InvalidArgument);
        }
    }

    auto node = area_list_.PushFront(vma);
    if (!node) {
        return unexpected(MemError::OutOfMemory);
    }

    return {};
}

bool AS::AreasOverlap(VPtr<VMemArea> a, VPtr<VMemArea> b)
{
    const auto a_s_addr = reinterpret_cast<uptr>(a->start);
    const auto a_e_addr = a_s_addr + a->size;
    const auto b_s_addr = reinterpret_cast<uptr>(b->start);
    const auto b_e_addr = b_s_addr + b->size;

    return a_s_addr < b_e_addr && b_s_addr < a_e_addr;
}

expected<TlbHint, MemError> AS::RmArea(VPtr<void> ptr)
{
    std::lock_guard guard(area_list_lock_);

    auto res = FindAreaLocked(ptr);
    RET_UNEXPECTED_IF_ERR(res);
    auto it = *res;

    // Do MMU unmap while we have the area info
    auto start = it->start;
    auto size  = it->size;

    mmu_->UnmapRange(*ctx_, page_table_root_, start, size);

    area_list_.Remove(it.GetNode());

    return TlbHint{start, size};
}

expected<TlbHint, MemError> AS::UpdateAreaFlags(VPtr<void> ptr, VirtualMemAreaFlags vmaf)
{
    std::lock_guard guard(area_list_lock_);

    auto res = FindAreaLocked(ptr);
    RET_UNEXPECTED_IF_ERR(res);
    VMemArea &area = *res.value();
    RET_UNEXPECTED_IF(area.start != ptr, MemError::InvalidArgument);

    area.flags = vmaf;
    auto start = area.start;
    auto size  = area.size;

    bool is_kernel = (PtrToUptr(start) >= hal::kKernelVirtualAddressStart);

    hal::PageFlags pf{
        .Present        = true,
        .Writable       = vmaf.writable,
        .UserAccessible = !is_kernel,
        .WriteThrough   = vmaf.write_through,
        .CacheDisable   = vmaf.cache_disable,
        .Global         = is_kernel,
        .NoExecute      = !vmaf.executable
    };

    uptr start_u = PtrToUptr(start);
    uptr end_u   = start_u + size;

    for (uptr v = start_u; v < end_u; v += hal::kPageSizeBytes) {
        auto res = mmu_->SetPageFlags(page_table_root_, UptrToPtr<void>(v), pf);
        RET_UNEXPECTED_IF(!res && res.error() != MemError::NotFound, res.error());
    }

    return TlbHint{start, size};
}

expected<AS::AddrSpMutIt, MemError> AS::FindAreaLocked(VPtr<void> ptr)
{
    for (auto it = area_list_.begin(); it != area_list_.end(); ++it) {
        if (IsAddrInArea(&(*it), ptr)) {
            return it;
        }
    }
    return unexpected(MemError::NotFound);
}

expected<VPtr<VMemArea>, MemError> AS::FindArea(VPtr<void> ptr)
{
    std::lock_guard guard(area_list_lock_);

    auto res = FindAreaLocked(ptr);
    RET_UNEXPECTED_IF_ERR(res);

    VPtr<VMemArea> vma = &(*res.value());
    return vma;
}

bool AS::IsAddrInArea(VPtr<VMemArea> vma, VPtr<void> ptr)
{
    ASSERT_NOT_NULL(vma, "Virtual memory area is null");

    const auto vma_s_addr = reinterpret_cast<uptr>(vma->start);
    const auto vma_e_addr = vma_s_addr + vma->size;
    const auto addr       = reinterpret_cast<uptr>(ptr);

    // Check if ptr is in [vma start, vma end]
    if (vma_s_addr <= addr && addr < vma_e_addr) {
        return true;
    }

    return false;
}
