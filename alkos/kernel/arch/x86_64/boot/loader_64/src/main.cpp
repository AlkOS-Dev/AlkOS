#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

#include <extensions/debug.hpp>
#include <extensions/expected.hpp>

#include "abi/transition_data.hpp"
#include "cpu/utils.hpp"
#include "elf/elf_64.hpp"
#include "elf/error.hpp"
#include "mem/page_map.hpp"
#include "mem/physical_ptr.hpp"
#include "mem/pmm.hpp"
#include "mem/vmm.hpp"
#include "multiboot2/info.hpp"
#include "multiboot2/multiboot2.h"
#include "settings.hpp"
#include "sys/panic.hpp"
#include "sys/terminal.hpp"

using namespace Multiboot;

//==============================================================================
// External Functions and Variables
//==============================================================================

BEGIN_DECL_C

void EnterKernel(u64 kernel_entry_addr, KernelArguments *kernel_initial_params);

// Defined in .ld
extern const char loader_64_start[];
extern const char loader_64_end[];

END_DECL_C

//==============================================================================
// Global Definitions
//==============================================================================

struct KernelModuleInfo {
    const TagModule *tag;
    u64 lower_bound;
    u64 upper_bound;
    u64 size;
};

struct MemoryManagers {
    PhysicalMemoryManager &pmm;
    VirtualMemoryManager &vmm;
};

//==============================================================================
// Global Data
//==============================================================================

static KernelArguments gKernelInitialParams;

alignas(
    PageSize<PageSizeTag::k4Kb>()
) static byte gPmmPreAllocatedMemory[sizeof(PhysicalMemoryManager)];
alignas(
    PageSize<PageSizeTag::k4Kb>()
) static byte gVmmPreAllocatedMemory[sizeof(VirtualMemoryManager)];

//==============================================================================
// High-Level Boot Steps
//==============================================================================

static std::tuple<MemoryManagers, MultibootInfo> InitializeLoaderEnvironment(
    const TransitionData *transition_data
)
{
    TerminalInit();
    TRACE_INFO("Loader 64-Bit Stage");
    ASSERT_NOT_NULL(transition_data, "TransitionData pointer is null");

    TRACE(
        "Transition Data:\n"
        "  multiboot_info_addr:         0x%llX\n"
        "  multiboot_header_start_addr: 0x%llX\n"
        "  multiboot_header_end_addr:   0x%llX\n"
        "  PMM State:\n"
        "    total_pages:               %llu\n"
        "    bitmap_addr:               0x%llX\n"
        "    iteration_index:           %llu\n"
        "    iteration_index_32:        %llu\n"
        "  VMM State:\n"
        "    pml_4_table_phys_addr:     0x%llX\n",
        transition_data->multiboot_info_addr, transition_data->multiboot_header_start_addr,
        transition_data->multiboot_header_end_addr, transition_data->pmm_state.total_pages,
        transition_data->pmm_state.bitmap_addr, transition_data->pmm_state.iteration_index,
        transition_data->pmm_state.iteration_index_32,
        transition_data->vmm_state.pml_4_table_phys_addr
    );

    TRACE_DEBUG("Deserializing Pmm and Vmm");

    PhysicalMemoryManager *pmm_ptr =
        new (gPmmPreAllocatedMemory) PhysicalMemoryManager(transition_data->pmm_state);
    auto &pmm = *pmm_ptr;

    VirtualMemoryManager *vmm_ptr =
        new (gVmmPreAllocatedMemory) VirtualMemoryManager(pmm, transition_data->vmm_state);
    auto &vmm = *vmm_ptr;

    MemoryManagers mms{.pmm = pmm, .vmm = vmm};

    MultibootInfo mb_info(transition_data->multiboot_info_addr);

    return {mms, mb_info};
}

static KernelModuleInfo AnalyzeKernelModule(
    MultibootInfo &multiboot_info, [[maybe_unused]] MemoryManagers mem_managers
)
{
    TRACE_DEBUG("Locating and analyzing kernel module...");

    auto kernel_module_res = multiboot_info.FindTag<TagModule>([](TagModule *tag) {
        return strcmp(tag->cmdline, kKernelModuleCmdline) == 0;
    });

    if (!kernel_module_res) {
        KernelPanicFormat(
            "Could not find the '%s' module in multiboot tags!", kKernelModuleCmdline
        );
    }
    const TagModule *kernel_tag = kernel_module_res.value();

    auto bounds_res = Elf64::GetProgramBounds(reinterpret_cast<byte *>(kernel_tag->mod_start));
    ASSERT_TRUE(bounds_res, "Failed to get kernel ELF program bounds");

    auto [lower, upper] = bounds_res.value();
    return {kernel_tag, lower, upper, upper - lower};
}

static u64 LoadKernelIntoMemory(
    MultibootInfo &multiboot_info, const KernelModuleInfo &kernel_info, MemoryManagers mem_managers
)
{
    auto &vmm = mem_managers.vmm;

    TRACE_DEBUG("Preparing memory and loading kernel...");
    auto mmap_tag_res = multiboot_info.FindTag<TagMmap>();
    ASSERT_TRUE(mmap_tag_res, "Failed to find memory map tag in multiboot info");

    vmm.Alloc(kKernelVirtualAddressStart, kernel_info.size, kPresentBit | kWriteBit);

    // Load the ELF file from the module into the newly mapped memory
    byte *module_start   = reinterpret_cast<byte *>(kernel_info.tag->mod_start);
    auto entry_point_res = Elf64::Load(module_start);
    ASSERT_TRUE(entry_point_res, "Failed to load kernel ELF");

    return entry_point_res.value();
}

static void EstablishDirectMemMapping(MemoryManagers &mms)
{
    auto &vmm = mms.vmm;

    TRACE_DEBUG(
        "Creating direct memory mapping at 0x%llX with %llu Gb", kDirectMemMapAddrStart,
        kDirectMemMapSizeGb
    );

    vmm.Map<PageSizeTag::k1Gb>(
        kDirectMemMapAddrStart, 0, kDirectMemMapSizeGb * PageSize<PageSizeTag::k1Gb>(),
        kPresentBit | kWriteBit | kGlobalBit
    );
}

NO_RET static void TransitionToKernel(
    u64 kernel_entry_point, const TransitionData *transition_data,
    const KernelModuleInfo &kernel_info, MemoryManagers mem_managers
)
{
    auto &pmm = mem_managers.pmm;
    auto &vmm = mem_managers.vmm;

    TRACE_INFO("Preparing to transition to kernel...");

    // Prepare parameters for the kernel
    gKernelInitialParams.kernel_start_addr           = kernel_info.lower_bound;
    gKernelInitialParams.kernel_end_addr             = kernel_info.upper_bound;
    gKernelInitialParams.multiboot_info_addr         = transition_data->multiboot_info_addr;
    gKernelInitialParams.multiboot_header_start_addr = transition_data->multiboot_header_start_addr;
    gKernelInitialParams.multiboot_header_end_addr   = transition_data->multiboot_header_end_addr;

    gKernelInitialParams.mem_info_bitmap_addr  = pmm.GetBitmapAddress().Value();
    gKernelInitialParams.mem_info_total_pages  = pmm.GetTotalPages();
    gKernelInitialParams.pml_4_table_phys_addr = vmm.GetPml4Table().Value();

    const u64 ld_start_addr    = reinterpret_cast<u64>(loader_64_start);
    const u64 ld_end_addr      = reinterpret_cast<u64>(loader_64_end);
    const u64 al_ld_start_addr = AlignDown(ld_start_addr, PageSize<PageSizeTag::k4Kb>());
    const u64 al_ld_span = AlignUp(ld_end_addr - ld_start_addr, PageSize<PageSizeTag::k4Kb>());
    pmm.Free(PhysicalPtr<void>(al_ld_start_addr), al_ld_span);

    TRACE_INFO("Transitioning to kernel with parameters:");
    TRACE(
        "  Kernel Arguments:\n"
        "    kernel_start_addr:           0x%llX\n"
        "    kernel_end_addr:             0x%llX\n"
        "    pml_4_table_phys_addr:       0x%llX\n"
        "    mem_info_bitmap_addr:        0x%llX\n"
        "    mem_info_total_pages:        %llu\n"
        "    multiboot_info_addr:         0x%llX\n"
        "    multiboot_header_start_addr: 0x%llX\n"
        "    multiboot_header_end_addr:   0x%llX\n",
        gKernelInitialParams.kernel_start_addr, gKernelInitialParams.kernel_end_addr,
        gKernelInitialParams.pml_4_table_phys_addr, gKernelInitialParams.mem_info_bitmap_addr,
        gKernelInitialParams.mem_info_total_pages, gKernelInitialParams.multiboot_info_addr,
        gKernelInitialParams.multiboot_header_start_addr,
        gKernelInitialParams.multiboot_header_end_addr
    );

    TRACE_INFO("Jumping to kernel at entry point: 0x%llX", kernel_entry_point);
    EnterKernel(kernel_entry_point, &gKernelInitialParams);

    KernelPanic("EnterKernel should not return!");
}

//==============================================================================
// Main 64-bit Loader Entry Point
//==============================================================================

extern "C" void MainLoader64(TransitionData *transition_data)
{
    auto [mms, multiboot_info] = InitializeLoaderEnvironment(transition_data);

    KernelModuleInfo kernel_info = AnalyzeKernelModule(multiboot_info, mms);

    u64 kernel_entry_point = LoadKernelIntoMemory(multiboot_info, kernel_info, mms);

    EstablishDirectMemMapping(mms);

    TransitionToKernel(kernel_entry_point, transition_data, kernel_info, mms);
}
