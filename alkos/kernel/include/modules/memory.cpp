#include "modules/memory.hpp"

#include <extensions/debug.hpp>

#include "hal/constants.hpp"
#include "kernel_args.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

using namespace mem;

static BitmapPmm InitBitMapPmm(const KernelArguments &args)
{
    const size_t total_pages          = args.total_page_frames;
    const VirtualPtr<void> mem_bitmap = args.mem_bitmap.ToVirt();

    return BitmapPmm{mem_bitmap, total_pages};
}

internal::MemoryModule::MemoryModule(const KernelArguments &args) noexcept
    : BitmapPmm_{InitBitMapPmm(args)}
{
    TRACE_INFO("MemoryModule::MemoryModule()");

    const size_t total_pages    = args.total_page_frames;
    const size_t required_size  = PageMetaTable::CalcRequiredSize(total_pages);
    const size_t required_pages = required_size / hal::kPageSizeBytes;

    TRACE_DEBUG("Total  : | %llu | pages", total_pages);
    TRACE_DEBUG("Finding: | %llu | pages for page metadata table", required_pages);
    auto res = BitmapPmm_.Alloc(BitmapPmm::AllocationRequest{.num_pages = required_pages});
    R_ASSERT_TRUE(res, "Failed to find memory region large enough to contain map metatada");

    auto page_metas = reinterpret_cast<VirtualPtr<PageMeta<Dummy>>>((*res).ToVirt());
    PageMetaTable_  = PageMetaTable(page_metas, total_pages);
}
