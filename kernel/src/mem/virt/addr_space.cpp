#include "mem/virt/addr_space.hpp"
#include "mem/error.hpp"
#include "mem/heap.hpp"
#include "mem/types.hpp"

using namespace Mem;
using AS = AddressSpace;

AS::~AddressSpace()
{
    // Free all allocated areas
    while (area_list_head_) {
        auto to_free    = area_list_head_;
        area_list_head_ = area_list_head_->next;
        KFree(to_free);
    }
}

Expected<void, MemError> AS::AddArea(VMemArea vma)
{
    auto res = KMalloc<VMemArea>();
    UNEXPECTED_RET_IF_ERR(res);

    VPtr<VMemArea> n_area = *res;
    *n_area               = vma;
    n_area->next          = nullptr;

    area_list_lock_.Lock();

    // Check for overlapping areas
    for (auto it = area_list_head_; it; it = it->next) {
        if (AreasOverlap(it, n_area)) {
            area_list_lock_.Unlock();
            KFree(n_area);
            return Unexpected(MemError::InvalidArgument);
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

Expected<void, MemError> AS::RmArea(VPtr<void> ptr)
{
    area_list_lock_.Lock();

    if (!area_list_head_) {
        area_list_lock_.Unlock();
        return {};  // Nothing to remove
    }

    // Head is to be removed
    if (IsAddrInArea(area_list_head_, ptr)) {
        auto to_free    = area_list_head_;
        area_list_head_ = area_list_head_->next;
        area_list_lock_.Unlock();
        KFree(to_free);
        return {};
    }

    // Traverse rest
    auto iterator = area_list_head_;
    while (iterator->next) {
        if (IsAddrInArea(iterator->next, ptr)) {
            auto to_free   = iterator->next;
            iterator->next = to_free->next;
            area_list_lock_.Unlock();
            KFree(to_free);
            return {};
        }
        iterator = iterator->next;
    }

    area_list_lock_.Unlock();
    return Unexpected(MemError::InvalidArgument);
}

Expected<VPtr<VMemArea>, MemError> AS::FindArea(VPtr<void> ptr)
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
    return Unexpected(MemError::NotFound);
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
