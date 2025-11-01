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

    FAST_CALL PageMeta &GetPageMeta(size_t pfn)
    {
        ASSERT_TRUE(pfn < num_page_frames_, "Page frame number is out of bounds");
        return page_frames_metas_[pfn];
    }

    FAST_CALL const PageMeta &GetPageMeta(size_t pfn) const
    {
        ASSERT_TRUE(pfn < num_page_frames_, "Page frame number is out of bounds");
        return page_frames_metas_[pfn];
    }

    FAST_CALL size_t GetPageFrameNumber(const PageMeta *meta) const
    {
        ASSERT_TRUE(
            meta >= page_frames_metas_ && meta < page_frames_metas_ + num_page_frames_,
            "Meta pointer is out of bounds"
        );
        return meta - page_frames_metas_;
    }

    FAST_CALL size_t TotalPages() const { return num_page_frames_; }

    private:
    VPtr<PageMeta> page_frames_metas_;
    size_t num_page_frames_;
};
}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_
