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

    VPtr<PageMeta> freelist_table_[kMaxPageOrder + 1];
    PageMetaTable *pmt_{nullptr};
    Spinlock lock_{};
};

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BUDDY_HPP_
