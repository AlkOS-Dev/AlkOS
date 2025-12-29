#include "modules/memory.hpp"

#include <assert.h>
#include "boot_args.hpp"
#include "hal/constants.hpp"
#include "mem/heap.hpp"
#include "mem/init/boot_mmu.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/types.hpp"
#include "mem/virt/area.hpp"
#include "mem/virt/page_fault.hpp"
#include "trace_framework.hpp"

using namespace Mem;

internal::MemoryModule::MemoryModule(const BootArguments &args) noexcept
{
    DEBUG_INFO_MEMORY("MemoryModule::MemoryModule()");

    // Prepare
    const size_t total_pages    = args.total_page_frames;
    const VPtr<void> mem_bitmap = PhysToVirt(args.mem_bitmap);
    data_structures::BitMapView bmv{mem_bitmap, total_pages};

    // Init
    BitmapPmm_.Init(bmv);

    PageMetaTable_.Init(args.total_page_frames, BitmapPmm_);

    // Cleanup bootloader mappings and reconstruct metadata for kernel mappings
    Mem::Boot::BootMmuCleaner boot_cleaner;

    TRACE_INFO_MEMORY("Unmapping lower half of memory");
    boot_cleaner.CleanIdentityMappings(Mmu_, BitmapPmm_, PageMetaTable_, args.root_page_table);
    Tlb_.FlushAll();

    constexpr size_t kInitialBuddyPagesLimit = 4096;  // 16MB
    // Note: Initializing buddy with all pages is a slow operation.
    // This limit is for speed of boot. This operation should have
    // a follow up once kernel is booted, and we have another CPU, we
    // could offload this operation to.
    TRACE_INFO_MEMORY("Initializing Buddy PMM");
    BuddyPmm_.Init(BitmapPmm_, PageMetaTable_, kInitialBuddyPagesLimit);

    // Reconstruct metadata for the existing page table hierarchy passed by the bootloader.
    TRACE_INFO_MEMORY("Reconstructing page table metadata from root: 0x%p", args.root_page_table);
    boot_cleaner.ReconstructMetadata(Mmu_, PageMetaTable_, args.root_page_table);

    TRACE_INFO_MEMORY("Initializing Slab Allocator");
    SlabAllocator_.Init(BuddyPmm_);

    TRACE_INFO_MEMORY("Initializing Heap");
    Heap_.Init(PageMetaTable_, BuddyPmm_, SlabAllocator_);

    KernelMmuContext_.Init(&BuddyPmm_, &PageMetaTable_);

    TRACE_INFO_MEMORY("Initializing Virtual Memory Manager");
    Vmm_.Init(Tlb_, Mmu_, KernelMmuContext_, Heap_, args.root_page_table);
}

void internal::MemoryModule::RegisterKernelVMAreas(const BootArguments &args)
{
    DEBUG_INFO_MEMORY("Registering Kernel VMAreas...");

    // Kernel Image
    size_t kernel_size =
        reinterpret_cast<uptr>(args.kernel_end) - reinterpret_cast<uptr>(args.kernel_start);

    auto kernel_vma_res = Mem::KNew<Mem::AnonymousVMemArea>(
        args.kernel_start, kernel_size,
        VirtualMemAreaFlags{.readable = true, .writable = true, .executable = true}
    );
    R_ASSERT_TRUE(kernel_vma_res);

    if (auto res = Vmm_.AddArea(&Vmm_.GetKernelAddressSpace(), *kernel_vma_res); !res) {
        TRACE_WARN_MEMORY("Failed to register Kernel VMA");
    }

    // Direct Physical Map
    auto dm_vma_res = Mem::KNew<Mem::DirectMappingVMemArea>(
        Mem::UptrToPtr<void>(hal::kDirectMapAddrStart),
        hal::kDirectMemMapSizeGb * 1024ULL * 1024ULL * 1024ULL,
        VirtualMemAreaFlags{.readable = true, .writable = true, .executable = false},
        Mem::UptrToPtr<void>(0)  // phys_start
    );
    R_ASSERT_TRUE(dm_vma_res);

    if (auto res = Vmm_.AddArea(&Vmm_.GetKernelAddressSpace(), *dm_vma_res); !res) {
        TRACE_WARN_MEMORY("Failed to register Direct Map VMA");
    }
}

void internal::MemoryModule::RegisterPageFault(HardwareModule &hw)
{
    DEBUG_INFO_MEMORY("Registering Page Fault handler...");
    hw.GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kException>(
        hal::kPageFaultExcLirq, intr::ExcHandler{.handler = Mem::PageFaultHandler}
    );
}
