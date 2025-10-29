#include "modules/memory.hpp"

#include <extensions/debug.hpp>

#include "boot_args.hpp"
#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/types.hpp"
#include "mem/virt/page_fault.hpp"

using namespace Mem;

static BitmapPmm InitBitMapPmm(const BootArguments &args)
{
    const size_t total_pages    = args.total_page_frames;
    const VPtr<void> mem_bitmap = PhysToVirt(args.mem_bitmap);

    return BitmapPmm{mem_bitmap, total_pages};
}

internal::MemoryModule::MemoryModule(const BootArguments &args) noexcept
    : BitmapPmm_{InitBitMapPmm(args)}, Vmm_{Tlb_, Mmu_}, KernelAddressSpace_(args.root_page_table)
{
    TRACE_INFO("MemoryModule::MemoryModule()");

    PageMetaTable_.Init(args.total_page_frames, BitmapPmm_);
}

void internal::MemoryModule::RegisterPageFault(HardwareModule &hw)
{
    TRACE_INFO("Registering Page Fault handler...");
    hw.GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kException>(
        hal::kPageFaultExcLirq, intr::ExcHandler{.handler = Mem::PageFaultHandler}
    );
}
