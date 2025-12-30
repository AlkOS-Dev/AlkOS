#include "mem/virt/addr_space.hpp"

#include <macros.hpp>
#include <mutex.hpp>

#include <template/scope_guard.hpp>

#include "constants.hpp"
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
    // Manually free all VMA objects before the list destroys the nodes
    for (auto *vma : area_list_) {
        KDelete(vma);
    }
    area_list_.Clear();

    if (ctx_) {
        if (owns_page_table_root_ && page_table_root_ != nullptr) {
            ctx_->FreeTable(page_table_root_, 0);
        }
    }
}

expected<void, MemError> AS::AddArea(VMemArea *vma)
{
    ASSERT_NOT_NULL(vma);
    R_ASSERT_TRUE(IsAligned(vma->GetStart(), hal::kPageSizeBytes), "VMA start is not aligned");
    R_ASSERT_TRUE(IsAligned(vma->GetSize(), hal::kPageSizeBytes), "VMA size is not aligned");

    std::lock_guard guard(area_list_lock_);
    template_lib::ScopeGuard vma_guard([&] {
        KDelete(vma);
    });

    for (auto it = area_list_.begin(); it != area_list_.end(); ++it) {
        VMemArea *current = *it;
        RET_UNEXPECTED_IF(AreasOverlap(current, vma), MemError::InvalidArgument);

        if (vma->GetStart() < current->GetStart()) {
            auto *pos = it.GetNode();
            auto *new_node =
                pos->prev ? area_list_.InsertAfter(pos->prev, vma) : area_list_.PushFront(vma);

            RET_UNEXPECTED_IF(!new_node, MemError::OutOfMemory);
            vma_guard.dismiss();
            return {};
        }
    }

    RET_UNEXPECTED_IF(!area_list_.PushBack(vma), MemError::OutOfMemory);
    vma_guard.dismiss();

    return {};
}

bool AS::AreasOverlap(const VMemArea *a, const VMemArea *b)
{
    const auto a_s = reinterpret_cast<uptr>(a->GetStart());
    const auto a_e = a_s + a->GetSize();
    const auto b_s = reinterpret_cast<uptr>(b->GetStart());
    const auto b_e = b_s + b->GetSize();
    return a_s < b_e && b_s < a_e;
}

expected<TlbHint, MemError> AS::RmArea(VPtr<void> ptr)
{
    std::lock_guard guard(area_list_lock_);

    auto res = FindAreaLocked(ptr);
    RET_UNEXPECTED_IF_ERR(res);
    auto it       = *res;
    VMemArea *vma = *it;

    // Do MMU unmap
    auto start = vma->GetStart();
    auto size  = vma->GetSize();
    mmu_->UnmapRange(*ctx_, page_table_root_, start, size);

    // Remove from list and delete object
    area_list_.Remove(it.GetNode());
    KDelete(vma);

    return TlbHint{start, size};
}

expected<TlbHint, MemError> AS::UpdateAreaFlags(VPtr<void> ptr, VirtualMemAreaFlags vmaf)
{
    std::lock_guard guard(area_list_lock_);

    auto res = FindAreaLocked(ptr);
    RET_UNEXPECTED_IF_ERR(res);
    VMemArea *vma = *res.value();

    RET_UNEXPECTED_IF(vma->GetStart() != ptr, MemError::InvalidArgument);

    vma->SetFlags(vmaf);

    auto start = vma->GetStart();
    auto size  = vma->GetSize();

    bool is_kernel = IsKernelSpace(start);

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
        if (IsAddrInArea(*it, ptr)) {
            return it;
        }
    }
    return unexpected(MemError::NotFound);
}

expected<VMemArea *, MemError> AS::FindArea(VPtr<void> ptr)
{
    std::lock_guard guard(area_list_lock_);
    auto res = FindAreaLocked(ptr);
    RET_UNEXPECTED_IF_ERR(res);
    return *(*res);
}

bool AS::IsAddrInArea(const VMemArea *vma, VPtr<void> ptr)
{
    ASSERT_NOT_NULL(vma);

    const auto vma_s = reinterpret_cast<uptr>(vma->GetStart());
    const auto vma_e = vma_s + vma->GetSize();
    const auto addr  = reinterpret_cast<uptr>(ptr);
    return vma_s <= addr && addr < vma_e;
}

expected<GapInfo, MemError> AS::FindGap(size_t size, VPtr<void> range_start, VPtr<void> range_end)
{
    std::lock_guard guard(area_list_lock_);

    uptr search_start = range_start ? PtrToUptr(range_start) : 0;
    uptr search_end   = range_end ? PtrToUptr(range_end) : UINTPTR_MAX;

    search_start = AlignUp(search_start, hal::kPageSizeBytes);
    search_end   = AlignDown(search_end, hal::kPageSizeBytes);

    uptr prev_end = search_start;

    for (auto *vma : area_list_) {
        uptr curr_start = PtrToUptr(vma->GetStart());
        uptr curr_end   = curr_start + vma->GetSize();

        // Search only in the range of interest
        if (curr_end <= search_start) {
            // Handle case where VMA overlaps the search_start boundary but ends after it
            prev_end = std::max(prev_end, curr_end);
            continue;
        }
        if (curr_start >= search_end) {
            break;
        }

        // Gap found
        uptr gap_start = AlignUp(prev_end, hal::kPageSizeBytes);
        uptr gap_end   = AlignDown(curr_start, hal::kPageSizeBytes);

        if (gap_end > gap_start && (gap_end - gap_start) >= size) {
            return GapInfo{UptrToPtr<void>(gap_start), size};
        }

        prev_end = curr_end;
    }

    uptr gap_start = AlignUp(prev_end, hal::kPageSizeBytes);

    if (search_end > gap_start && (search_end - gap_start) >= size) {
        return GapInfo{UptrToPtr<void>(gap_start), size};
    }

    return unexpected(MemError::NotFound);
}
