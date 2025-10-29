#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_

#include "mem/page_meta.hpp"
#include "mem/types.hpp"

namespace Mem
{

class BitmapPmm;

class PageMetaTable
{
    public:
    PageMetaTable()  = default;
    ~PageMetaTable() = default;

    void Init(size_t total_pages, BitmapPmm &b_pmm);

    private:
    VPtr<PageMeta<Dummy>> page_frames_metas_;
    size_t num_page_frames_;
};
}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_
