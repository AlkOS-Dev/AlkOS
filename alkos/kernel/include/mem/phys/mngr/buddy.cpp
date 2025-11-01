#include <extensions/algorithm.hpp>
#include <extensions/mutex.hpp>

#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/phys/mngr/buddy.hpp"

using namespace Mem;
using B = BuddyPmm;

void B::ListRemove(PageMeta *meta)
{
    auto &buddy_meta = PageMeta::AsBuddy(*meta);
    if (buddy_meta.prev) {
        PageMeta::AsBuddy(*buddy_meta.prev).next = buddy_meta.next;
    }
    if (buddy_meta.next) {
        PageMeta::AsBuddy(*buddy_meta.next).prev = buddy_meta.prev;
    }

    if (freelist_table_[meta->order] == meta) {
        freelist_table_[meta->order] = buddy_meta.next;
    }
}

void B::ListPush(PageMeta *meta)
{
    auto &head = freelist_table_[meta->order];

    auto &buddy_meta = PageMeta::AsBuddy(*meta);
    buddy_meta.next  = head;
    if (head) {
        PageMeta::AsBuddy(*head).prev = meta;
    }
    head = meta;
}

void B::Init(BitmapPmm &b_pmm, PageMetaTable &pmt)
{
    auto bitmap_view = b_pmm.GetBitmapView();
    pmt_             = &pmt;

    for (size_t i = 0; i < kMaxPageOrder + 1; i++) {
        freelist_table_[i] = nullptr;
    }

    for (size_t i = 0; i < bitmap_view.Size(); i++) {
        auto &meta = pmt.GetPageMeta(i);
        meta.InitAllocated(0);
    }

    for (size_t i = 0; i < bitmap_view.Size(); i++) {
        if (b_pmm.IsFree(i)) {
            Free(PageFrameAddr(i));
        }
    }
}

void B::Free(PPtr<Page> page)
{
    std::lock_guard guard{lock_};
    size_t pfn     = PageFrameNumber(page);
    PageMeta &meta = pmt_->GetPageMeta(pfn);

    ASSERT_EQ(meta.type, PageMetaType::Allocated);
    u8 order = meta.order;

    while (order < kMaxPageOrder) {
        size_t buddy_pfn = pfn ^ (1 << order);

        if (buddy_pfn >= pmt_->TotalPages()) {
            break;
        }
        PageMeta &buddy_meta = pmt_->GetPageMeta(buddy_pfn);

        if (buddy_meta.type == PageMetaType::Buddy && buddy_meta.order == order) {
            ListRemove(&buddy_meta);
            pfn = std::min(pfn, buddy_pfn);
            order++;
        } else {
            break;
        }
    }

    pmt_->GetPageMeta(pfn).InitBuddy(order);
    ListPush(&pmt_->GetPageMeta(pfn));
}

Expected<PPtr<Page>, MemError> B::Alloc(AllocationRequest ar)
{
    std::lock_guard guard{lock_};
    u8 order = ar.order;

    if (order > kMaxPageOrder) {
        return MemError::OutOfMemory;
    }

    for (u8 current_order = order; current_order <= kMaxPageOrder; current_order++) {
        if (freelist_table_[current_order]) {
            PageMeta *block = freelist_table_[current_order];
            ListRemove(block);

            size_t pfn = pmt_->GetPageFrameNumber(block);
            for (u8 i = current_order; i > order; i--) {
                u8 lower_order       = i - 1;
                size_t buddy_pfn     = pfn + (1 << lower_order);
                PageMeta &buddy_meta = pmt_->GetPageMeta(buddy_pfn);
                buddy_meta.InitBuddy(lower_order);
                ListPush(&buddy_meta);
            }

            block->InitAllocated(order);
            return PageFrameAddr(pfn);
        }
    }

    return MemError::OutOfMemory;
}
