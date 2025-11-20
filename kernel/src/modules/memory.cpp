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

    BuddyPmm_.Init(BitmapPmm_, PageMetaTable_);

    SlabAllocator_.Init(BuddyPmm_);

    Vmm_.Init(Tlb_, Mmu_);
}

void internal::MemoryModule::RegisterPageFault(HardwareModule &hw)
{
    DEBUG_INFO_MEMORY("Registering Page Fault handler...");
    hw.GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kException>(
        hal::kPageFaultExcLirq, intr::ExcHandler{.handler = Mem::PageFaultHandler}
    );
}
