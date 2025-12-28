#include "modules/memory.hpp"

#include "boot_args.hpp"
#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/types.hpp"
#include "mem/virt/page_fault.hpp"
#include "trace_framework.hpp"

using namespace Mem;

internal::MemoryModule::MemoryModule(const BootArguments &args) noexcept
    : KernelAddressSpace_(args.root_page_table)
{
    DEBUG_INFO_MEMORY("MemoryModule::MemoryModule()");

    // Prepare
    const size_t total_pages    = args.total_page_frames;
    const VPtr<void> mem_bitmap = PhysToVirt(args.mem_bitmap);
    data_structures::BitMapView bmv{mem_bitmap, total_pages};

    // Init
    BitmapPmm_.Init(bmv);

    PageMetaTable_.Init(args.total_page_frames, BitmapPmm_);

    constexpr size_t kInitialBuddyPagesLimit = 4096;  // 16MB
    // Note: Initializing buddy with all pages is a slow operation.
    // This limit is for speed of boot. This operation should have
    // a follow up once kernel is booted, and we have another CPU, we
    // could offload this operation to.
    BuddyPmm_.Init(BitmapPmm_, PageMetaTable_, kInitialBuddyPagesLimit);

    // Initialize metadata for the Kernel PML4 (Root Page Table).
    // The bootloader passes it, so it's already allocated but its metadata
    // needs to be correctly set as a PageTable to support MMU operations.
    {
        auto &root_meta = PageMetaTable_.GetPageMeta(args.root_page_table);

        // Initialize as a Level 4 page table with no parent.
        // We set ref_count to 1 to represent the kernel's static presence and prevent freeing.
        root_meta.InitPageTable(4, nullptr);
        root_meta.data.page_table.ref_count = 1;
    }

    SlabAllocator_.Init(BuddyPmm_);

    Heap_.Init(PageMetaTable_, BuddyPmm_, SlabAllocator_);

    Vmm_.Init(Tlb_, Mmu_);
}

void internal::MemoryModule::RegisterPageFault(HardwareModule &hw)
{
    DEBUG_INFO_MEMORY("Registering Page Fault handler...");
    hw.GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kException>(
        hal::kPageFaultExcLirq, intr::ExcHandler{.handler = Mem::PageFaultHandler}
    );
}
