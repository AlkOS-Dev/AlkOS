#include "mem/page_meta_table.hpp"
#include "hal/constants.hpp"
#include "mem/phys/mngr/bitmap.hpp"

using namespace Mem;
using enum PageMetaType;
using P = PageMetaTable;

void P::Init(size_t total_pages, BitmapPmm &b_pmm)
{
    TRACE_DEBUG("PageMetaTable::Init()");

    auto required_size_bytes    = total_pages * sizeof(PageMeta);
    const size_t required_pages = required_size_bytes / hal::kPageSizeBytes;

    TRACE_DEBUG("Total  : | %llu | pages", total_pages);
    TRACE_DEBUG("Finding: | %llu | pages for page metadata table", required_pages);

    auto res = b_pmm.Alloc(BitmapPmm::AllocationRequest{.num_pages = required_pages});
    R_ASSERT_TRUE(res, "Failed to allocate memory for page metadata table");

    auto *page_metas = reinterpret_cast<VPtr<PageMeta>>(PhysToVirt(*res));

    num_page_frames_   = total_pages;
    page_frames_metas_ = page_metas;
}
