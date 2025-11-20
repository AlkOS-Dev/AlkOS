#ifndef KERNEL_SRC_MEM_PHYS_MNGR_BUDDY_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_BUDDY_HPP_

#include <types.hpp>

#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/page_meta.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/types.hpp"
#include "sync/kernel/spinlock.hpp"

// Forward declaration for test access (test class is in global namespace)
class BuddyPmmTest;

namespace Mem
{

class BitmapPmm;

class BuddyPmm
{
    public:
    /// MaxOrder of 10 means that 2^0 num pages are allowed up to 2^10 (order 10)
    /// and system will try to merge into continous areas as such
    static constexpr u8 kMaxOrder = 10;
    struct AllocationRequest {
        u8 order = 0;
    };

    BuddyPmm();

    static constexpr size_t kNoPageLimit = -1;
    void Init(BitmapPmm &b_pmm, PageMetaTable &pmt, size_t page_limit = kNoPageLimit);

    Expected<PPtr<Page>, MemError> Alloc(AllocationRequest ar);
    void Free(PPtr<Page> page);

    static constexpr size_t BuddyAreaSize(u8 order)
    {
        // PageSize * 2 ^ order
        return static_cast<size_t>(hal::kPageSizeBytes) << order;
    }

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

    /**
     * @brief Private accessor for test/debug use.
     * @param order The freelist order to query.
     * @return Pointer to the head of the freelist for the given order.
     */
    VPtr<PageMeta> GetFreelistHead(u8 order) const { return freelist_table_[order]; }

    VPtr<PageMeta> freelist_table_[kMaxOrder + 1];
    Spinlock lock_{};

    /// Dependencies
    PageMetaTable *pmt_{nullptr};

    /// Friends
    friend ::BuddyPmmTest;
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PHYS_MNGR_BUDDY_HPP_
