#include "modules/memory.hpp"

#include "boot_args.hpp"
#include "hal/constants.hpp"
#include "mem/init/boot_mmu.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/types.hpp"
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

    KernelAddressSpace_.InitKernel(args.root_page_table, KernelMmuContext_, Mmu_);

    TRACE_INFO_MEMORY("Initializing Virtual Memory Manager");
    Vmm_.Init(Tlb_, Mmu_, KernelMmuContext_, Heap_);

    TRACE_INFO_MEMORY("Initializing Kernel Address Space");

    // Register initial Virtual Memory Areas (VMAs) so the VMM is aware of them.
    // These areas were set up by the bootloader but are invisible to the generic VMM until
    // registered.
    // {
    //     // Kernel Image
    //     size_t kernel_size =
    //         reinterpret_cast<uptr>(args.kernel_end) - reinterpret_cast<uptr>(args.kernel_start);
    //
    //     VMemArea kernel_vma{
    //         .start                = args.kernel_start,
    //         .size                 = kernel_size,
    //         .flags                = {.readable = true, .writable = true, .executable = true},
    //         .type                 = VirtualMemAreaT::Anonymous,
    //         .direct_mapping_start = VirtToPhys(args.kernel_start),
    //         .next                 = nullptr
    //     };

    //     if (auto res = Vmm_.AddArea(&KernelAddressSpace_, kernel_vma); !res) {
    //         TRACE_WARN_MEMORY("Failed to register Kernel VMA");
    //     }

    //     // Direct Physical Map
    //     VMemArea dm_vma{
    //         .start                = UptrToPtr<void>(hal::kDirectMapAddrStart),
    //         .size                 = hal::kDirectMemMapSizeGb * 1024ULL * 1024ULL * 1024ULL,
    //         .flags                = {.readable = true, .writable = true, .executable = false},
    //         .type                 = VirtualMemAreaT::DirectMapping,
    //         .direct_mapping_start = UptrToPtr<void>(0),
    //         .next                 = nullptr
    //     };

    //     if (auto res = Vmm_.AddArea(&KernelAddressSpace_, dm_vma); !res) {
    //         TRACE_WARN_MEMORY("Failed to register Direct Map VMA");
    //     }
    //
    // }
}

void internal::MemoryModule::RegisterPageFault(HardwareModule &hw)
{
    DEBUG_INFO_MEMORY("Registering Page Fault handler...");
    hw.GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kException>(
        hal::kPageFaultExcLirq, intr::ExcHandler{.handler = Mem::PageFaultHandler}
    );
}
