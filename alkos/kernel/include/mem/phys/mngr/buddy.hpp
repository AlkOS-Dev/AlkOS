#ifndef ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BUDDY_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BUDDY_HPP_

#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/page.hpp"
#include "mem/page_meta.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/types.hpp"

namespace Mem
{

class BitmapPmm;

class BuddyPmm
{
    private:
    static constexpr u8 kMaxPageOrder = 10;

    public:
    struct AllocationRequest {
        size_t num_pages = 1;
    };

    BuddyPmm();

    void Init(BitmapPmm &b_pmm);

    Expected<PPtr<Page>, MemError> Alloc(AllocationRequest ar = {.num_pages = 1});
    void Free(PPtr<Page> page, size_t num_pages = 1);

    private:
    PageMeta<Buddy> *freelist_table[kMaxPageOrder];
};

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PHYS_MNGR_BUDDY_HPP_
