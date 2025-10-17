#include "modules/memory.hpp"

#include <extensions/debug.hpp>

#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

using namespace mem;

internal::MemoryModule::MemoryModule(const hal::KernelArguments &args) noexcept
{
    TRACE_INFO("MemoryModule::MemoryModule()");

    const size_t total_pages               = static_cast<size_t>(args.mem_info_total_pages);
    const VirtualPtr<void> mem_bitmap_addr = reinterpret_cast<void *>(args.mem_info_bitmap_addr);

    BitmapPmm b_pmm{mem_bitmap_addr, total_pages};

    const size_t required_size  = PageMetaTable::CalcRequiredSize(total_pages);
    const size_t required_pages = required_size / hal::kPageSizeBytes;

    TRACE_DEBUG("Finding: | %llu | pages", required_pages);
    TRACE_DEBUG("Total  : | %llu | pages", total_pages);
    auto res = b_pmm.Alloc(BitmapPmm::AllocationRequest{.num_pages = required_pages});
    R_ASSERT_TRUE(res, "Failed to find memory region large enough to contain map metatada");

    auto page_metas = reinterpret_cast<VirtualPtr<PageMeta<Dummy>>>((*res).Get());
    PageMetaTable_  = PageMetaTable(page_metas, total_pages);
}
