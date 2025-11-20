#include "mem/phys/mngr/buddy.hpp"

#include <algorithm.hpp>
#include <mutex.hpp>

#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"

using namespace Mem;
using B = BuddyPmm;

//==============================================================================
// Private Helper Methods
//==============================================================================

size_t B::GetBuddyPfn(size_t pfn, u8 order)
{
    // The buddy's PFN is found by XORing the current PFN with the block size (2^order).
    return pfn ^ (1 << order);
}

void B::ListRemove(PageMeta *meta)
{
    auto &buddy_meta = PageMeta::AsBuddy(*meta);

    if (buddy_meta.next != nullptr) {
        PageMeta::AsBuddy(*buddy_meta.next).prev = buddy_meta.prev;
    }
    if (buddy_meta.prev != nullptr) {
        PageMeta::AsBuddy(*buddy_meta.prev).next = buddy_meta.next;
    }

    if (freelist_table_[meta->order] == meta) {
        freelist_table_[meta->order] = buddy_meta.next;
    }

    buddy_meta.next = nullptr;
    buddy_meta.prev = nullptr;
}

void B::ListPush(PageMeta *meta)
{
    auto &head       = freelist_table_[meta->order];
    auto &buddy_meta = PageMeta::AsBuddy(*meta);

    buddy_meta.next = head;
    if (head != nullptr) {
        PageMeta::AsBuddy(*head).prev = meta;
    }
    buddy_meta.prev = nullptr;
    head            = meta;
}

PageMeta *B::SplitBlock(PageMeta *block_to_split, u8 target_order)
{
    u8 current_order = block_to_split->order;
    size_t pfn       = pmt_->GetPageFrameNumber(block_to_split);

    ASSERT_GT(current_order, target_order, "SplitBlock called on a block that isn't larger");

    for (u8 i = current_order; i > target_order; i--) {
        u8 lower_order   = i - 1;
        size_t buddy_pfn = GetBuddyPfn(pfn, lower_order);

        // The higher-address buddy is always split off and added to the freelist
        PageMeta &buddy_meta = pmt_->GetPageMeta(buddy_pfn);
        buddy_meta.InitBuddy(lower_order);
        ListPush(&buddy_meta);
    }

    PageMeta &final_block = pmt_->GetPageMeta(pfn);
    final_block.order     = target_order;
    return &final_block;
}

PageMeta *B::MergeBlock(PageMeta *block_to_merge)
{
    u8 order   = block_to_merge->order;
    size_t pfn = pmt_->GetPageFrameNumber(block_to_merge);

    while (order < kMaxOrder) {
        size_t buddy_pfn = GetBuddyPfn(pfn, order);

        if (buddy_pfn >= pmt_->TotalPages()) {
            break;
        }

        PageMeta &buddy_meta = pmt_->GetPageMeta(buddy_pfn);

        // Check if the buddy is also free and has the same order
        if (buddy_meta.type == PageMetaType::Buddy && buddy_meta.order == order) {
            ListRemove(&buddy_meta);

            pfn = std::min(pfn, buddy_pfn);
            order++;

            // Invalidate the old buddy's metadata to prevent dangling state
            buddy_meta.type = PageMetaType::Dummy;
        } else {
            break;
        }
    }

    PageMeta &final_block = pmt_->GetPageMeta(pfn);
    final_block.order     = order;
    return &final_block;
}

//==============================================================================
// Public Methods
//==============================================================================

B::BuddyPmm() = default;

void B::Init(BitmapPmm &b_pmm, PageMetaTable &pmt, size_t page_limit)
{
    pmt_ = &pmt;

    for (size_t i = 0; i <= kMaxOrder; i++) {
        freelist_table_[i] = nullptr;
    }

    // Init all metatadata in allocated state
    for (size_t i = 0; i < pmt.TotalPages(); i++) {
        auto &meta = pmt.GetPageMeta(i);
        meta.InitAllocated(0);
    }

    // Works because of merging strat
    size_t pages_freed = 0;
    for (size_t i = 0; i < pmt.TotalPages() && pages_freed < page_limit; i++) {
        if (b_pmm.IsFree(i)) {
            Free(PageFrameAddr(i));
            pages_freed++;
        }
    }
}

Expected<PPtr<Page>, MemError> B::Alloc(AllocationRequest ar)
{
    std::lock_guard guard{lock_};
    u8 order = ar.order;

    if (order > kMaxOrder) {
        return Unexpected(MemError::InvalidArgument);
    }

    // Find the smallest available block that is large enough
    for (u8 current_order = order; current_order <= kMaxOrder; current_order++) {
        if (freelist_table_[current_order] != nullptr) {
            PageMeta *block_to_alloc = freelist_table_[current_order];
            ListRemove(block_to_alloc);

            if (current_order > order) {
                block_to_alloc = SplitBlock(block_to_alloc, order);
            }

            block_to_alloc->InitAllocated(order);
            return PageFrameAddr(pmt_->GetPageFrameNumber(block_to_alloc));
        }
    }

    return Unexpected(MemError::OutOfMemory);
}

void B::Free(PPtr<Page> page)
{
    std::lock_guard guard{lock_};
    size_t pfn     = PageFrameNumber(page);
    PageMeta &meta = pmt_->GetPageMeta(pfn);

    ASSERT_EQ(
        meta.type, PageMetaType::Allocated, "Double free detected or freeing an invalid page!"
    );

    PageMeta *merged_block = MergeBlock(&meta);
    merged_block->InitBuddy(merged_block->order);
    ListPush(merged_block);
}
