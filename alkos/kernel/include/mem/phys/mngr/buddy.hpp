#ifndef ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BUDDY_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BUDDY_HPP_

#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/page_meta.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/types.hpp"
#include "sync/kernel/spinlock.hpp"

namespace Mem
{

class BitmapPmm;

class BuddyPmm
{
    private:
    static constexpr u8 kMaxPageOrder = 10;

    public:
    struct AllocationRequest {
        u8 order = 0;
    };

    BuddyPmm();

    void Init(BitmapPmm &b_pmm, PageMetaTable &pmt);

    Expected<PPtr<Page>, MemError> Alloc(AllocationRequest ar);
    void Free(PPtr<Page> page);

    private:
    void ListRemove(PageMeta *meta);
    void ListPush(PageMeta *meta);
    static size_t GetBuddyPfn(size_t pfn, u8 order);

    /**
     * @brief Splits a large block until a block of target_order is created.
     * @param block_to_split Block to split. MUST NOT be on a freelist.
     * @param target_order The desired final order.
     * @return A pointer to the final, correctly-sized block.
     */
    PageMeta *SplitBlock(PageMeta *block_to_split, u8 target_order);

    /**
     * @brief Coalesces a free block with its buddies recursively.
     * @param block_to_merge Block to merge. MUST NOT be on a freelist.
     * @return A pointer to the final, largest possible merged block.
     */
    PageMeta *MergeBlock(PageMeta *block_to_merge);

    VPtr<PageMeta> freelist_table_[kMaxPageOrder + 1];
    Spinlock lock_{};

    /// Dependencies
    PageMetaTable *pmt_{nullptr};
};

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BUDDY_HPP_
