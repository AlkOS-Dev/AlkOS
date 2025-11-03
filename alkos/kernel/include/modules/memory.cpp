#include "modules/memory.hpp"

#include <extensions/debug.hpp>

#include "boot_args.hpp"
#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/types.hpp"
#include "mem/virt/page_fault.hpp"

using namespace Mem;

internal::MemoryModule::MemoryModule(const BootArguments &args) noexcept
    : KernelAddressSpace_(args.root_page_table)
{
    TRACE_INFO("MemoryModule::MemoryModule()");

    // Initialize BitmapPmm
    const size_t total_pages    = args.total_page_frames;
    const VPtr<void> mem_bitmap = PhysToVirt(args.mem_bitmap);
    data_structures::BitMapView bmv{mem_bitmap, total_pages};
    BitmapPmm_.Init(bmv);

    // Initialize PageMetaTable
    PageMetaTable_.Init(args.total_page_frames, BitmapPmm_);

    // Initialize BuddyPmm
    // BuddyPmm_.Init(BitmapPmm_, PageMetaTable_);

    // Initialize Vmm
    Vmm_.Init(Tlb_, Mmu_);
}

void internal::MemoryModule::RegisterPageFault(HardwareModule &hw)
{
    TRACE_INFO("Registering Page Fault handler...");
    hw.GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kException>(
        hal::kPageFaultExcLirq, intr::ExcHandler{.handler = Mem::PageFaultHandler}
    );
}
