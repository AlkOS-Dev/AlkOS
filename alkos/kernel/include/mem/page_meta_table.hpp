#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_

#include <extensions/span.hpp>
#include "hal/constants.hpp"
#include "mem/page.hpp"
#include "mem/page_meta.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/types.hpp"

namespace Mem
{

class BitmapPmm;

class PageMetaTable
{
    public:
    PageMetaTable()  = default;
    ~PageMetaTable() = default;

    inline void Init(size_t total_pages, BitmapPmm &b_pmm)
    {
        num_page_frames_               = total_pages;
        const size_t pmt_size          = num_page_frames_ * sizeof(PageMeta);
        const size_t num_pages_for_pmt = (pmt_size + hal::kPageSizeBytes - 1) / hal::kPageSizeBytes;

        auto pmt_ptr = b_pmm.Alloc({.num_pages = num_pages_for_pmt});
        ASSERT_TRUE(pmt_ptr.has_value(), "Failed to allocate memory for PageMetaTable");

        page_frames_metas_ = PhysToVirt(reinterpret_cast<PPtr<PageMeta>>(pmt_ptr.value()));
    }

    inline void Init(std::span<PageMeta> pmt_buffer)
    {
        num_page_frames_   = pmt_buffer.size();
        page_frames_metas_ = pmt_buffer.data();
    }

    PageMeta &GetPageMeta(size_t pfn)
    {
        ASSERT_TRUE(pfn < num_page_frames_, "Page frame number is out of bounds");
        return page_frames_metas_[pfn];
    }

    const PageMeta &GetPageMeta(size_t pfn) const
    {
        ASSERT_TRUE(pfn < num_page_frames_, "Page frame number is out of bounds");
        return page_frames_metas_[pfn];
    }

    size_t GetPageFrameNumber(const PageMeta *meta) const
    {
        ASSERT_TRUE(
            meta >= page_frames_metas_ && meta < page_frames_metas_ + num_page_frames_,
            "Meta pointer is out of bounds"
        );
        return meta - page_frames_metas_;
    }

    size_t TotalPages() const { return num_page_frames_; }

    private:
    VPtr<PageMeta> page_frames_metas_;
    size_t num_page_frames_;
};
}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_
