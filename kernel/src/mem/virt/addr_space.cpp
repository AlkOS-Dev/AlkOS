#include "mem/virt/addr_space.hpp"

#include <macros.hpp>
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

AS::AddressSpace()
    : page_table_root_{nullptr}, owns_page_table_root_{false}, area_list_head_{nullptr}
{
}

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
    while (area_list_head_) {
        auto to_free    = area_list_head_;
        area_list_head_ = area_list_head_->next;
        KFree(to_free);
    }

    if (ctx_) {
        if (owns_page_table_root_ && page_table_root_ != nullptr) {
            ctx_->FreeTable(page_table_root_, 0);
        }
    }
}

expected<void, MemError> AS::AddArea(VMemArea vma)
{
    auto res = KMalloc<VMemArea>();
    RET_UNEXPECTED_IF_ERR(res);

    VPtr<VMemArea> n_area = *res;
    *n_area               = vma;
    n_area->next          = nullptr;

    area_list_lock_.Lock();

    // Check for overlapping areas
    for (auto it = area_list_head_; it; it = it->next) {
        if (AreasOverlap(it, n_area)) {
            area_list_lock_.Unlock();
            KFree(n_area);
            return unexpected(MemError::InvalidArgument);
        }
    }

    n_area->next    = area_list_head_;
    area_list_head_ = n_area;

    area_list_lock_.Unlock();
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
    area_list_lock_.Lock();

    VPtr<VMemArea> to_free = nullptr;
    VPtr<VMemArea> prev    = nullptr;

    auto iterator = area_list_head_;
    while (iterator) {
        if (IsAddrInArea(iterator, ptr)) {
            to_free = iterator;
            break;
        }
        prev     = iterator;
        iterator = iterator->next;
    }

    if (!to_free) {
        area_list_lock_.Unlock();
        return unexpected(MemError::NotFound);
    }

    // Do MMU unmap while we have the area info
    auto start = to_free->start;
    auto size  = to_free->size;

    mmu_->UnmapRange(*ctx_, page_table_root_, start, size);

    if (prev) {
        prev->next = to_free->next;
    } else {
        area_list_head_ = to_free->next;
    }

    area_list_lock_.Unlock();
    KFree(to_free);

    return TlbHint{start, size};
}

expected<TlbHint, MemError> AS::UpdateAreaFlags(VPtr<void> ptr, VirtualMemAreaFlags vmaf)
{
    area_list_lock_.Lock();

    VPtr<VMemArea> area = area_list_head_;
    while (area != nullptr) {
        if (IsAddrInArea(area, ptr)) {
            break;
        }
        area = area->next;
    }

    if (!area) {
        area_list_lock_.Unlock();
        return unexpected(MemError::NotFound);
    }

    if (area->start != ptr) {
        area_list_lock_.Unlock();
        return unexpected(MemError::InvalidArgument);
    }

    area->flags = vmaf;
    auto start  = area->start;
    auto size   = area->size;

    bool is_kernel = (PtrToUptr(start) >= hal::kKernelVirtualAddressStart);

    hal::PageFlags pf{
        .Present        = true,
        .Writable       = vmaf.writable,
        .UserAccessible = !is_kernel,
        .WriteThrough   = false,
        .CacheDisable   = false,
        .Global         = is_kernel,
        .NoExecute      = !vmaf.executable
    };

    uptr start_u = PtrToUptr(start);
    uptr end_u   = start_u + size;

    for (uptr v = start_u; v < end_u; v += hal::kPageSizeBytes) {
        auto res = mmu_->SetPageFlags(page_table_root_, UptrToPtr<void>(v), pf);
        if (!res && res.error() != MemError::NotFound) {
            area_list_lock_.Unlock();
            return unexpected(res.error());
        }
    }

    area_list_lock_.Unlock();
    return TlbHint{start, size};
}

expected<VPtr<VMemArea>, MemError> AS::FindArea(VPtr<void> ptr)
{
    area_list_lock_.Lock();

    VPtr<VMemArea> vma = area_list_head_;
    while (vma != nullptr) {
        if (IsAddrInArea(vma, ptr)) {
            area_list_lock_.Unlock();
            return vma;
        }
        vma = vma->next;
    }

    area_list_lock_.Unlock();
    return unexpected(MemError::NotFound);
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
