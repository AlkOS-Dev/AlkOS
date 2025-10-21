#include "modules/memory.hpp"

#include <extensions/debug.hpp>

#include "boot_args.hpp"
#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/types.hpp"

using namespace Mem;

static BitmapPmm InitBitMapPmm(const BootArguments &args)
{
    const size_t total_pages    = args.total_page_frames;
    const VPtr<void> mem_bitmap = PhysToVirt(args.mem_bitmap);

    return BitmapPmm{mem_bitmap, total_pages};
}

internal::MemoryModule::MemoryModule(const BootArguments &args) noexcept
    : BitmapPmm_{InitBitMapPmm(args)}
{
    TRACE_INFO("MemoryModule::MemoryModule()");

    const size_t total_pages    = args.total_page_frames;
    const size_t required_size  = PageMetaTable::CalcRequiredSize(total_pages);
    const size_t required_pages = required_size / hal::kPageSizeBytes;

    TRACE_DEBUG("Total  : | %llu | pages", total_pages);
    TRACE_DEBUG("Finding: | %llu | pages for page metadata table", required_pages);
    auto res = BitmapPmm_.Alloc(BitmapPmm::AllocationRequest{.num_pages = required_pages});
    R_ASSERT_TRUE(res, "Failed to allocate memory for page metadata table");

    auto page_metas = reinterpret_cast<VPtr<PageMeta<Dummy>>>(PhysToVirt(*res));
    PageMetaTable_  = PageMetaTable(page_metas, total_pages);
}
