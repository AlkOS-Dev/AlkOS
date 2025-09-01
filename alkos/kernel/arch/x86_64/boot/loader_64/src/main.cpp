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
#include "mem/memory_manager.hpp"
#include "multiboot2/info.hpp"
#include "multiboot2/multiboot2.h"
#include "settings.hpp"
#include "sys/panic.hpp"
#include "sys/terminal.hpp"

using namespace Multiboot;

//==============================================================================
// External Functions and Variables
//==============================================================================

extern "C" void EnterKernel(u64 kernel_entry_addr, KernelInitialParams* kernel_initial_params);
extern const char loader_64_start[];
extern const char loader_64_end[];

//==============================================================================
// Global Data
//==============================================================================

static KernelInitialParams kernel_initial_params;

struct KernelModuleInfo {
    const TagModule* tag;
    u64 lower_bound;
    u64 upper_bound;
    u64 size;
};

//==============================================================================
// High-Level Boot Steps
//==============================================================================

static void InitializeLoaderEnvironment(const TransitionData* transition_data)
{
    TerminalInit();
    TRACE_INFO("Loader 64-Bit Stage");
    if (transition_data == nullptr) {
        KernelPanic("TransitionData validation failed: data is null!");
    }
}

static KernelModuleInfo AnalyzeKernelModule(MultibootInfo& multiboot_info)
{
    TRACE_DEBUG("Locating and analyzing kernel module...");

    auto kernel_module_res = multiboot_info.FindTag<TagModule>([](TagModule* tag) {
        return strcmp(tag->cmdline, "kernel") == 0;
    });

    if (!kernel_module_res) {
        KernelPanic("Could not find the 'kernel' module in multiboot tags!");
    }
    const TagModule* kernel_tag = kernel_module_res.value();

    auto bounds_res = Elf64::GetProgramBounds(reinterpret_cast<byte*>(kernel_tag->mod_start));
    if (!bounds_res) {
        KernelPanic("Failed to get kernel ELF program bounds!");
    }

    auto [lower, upper] = bounds_res.value();
    return {kernel_tag, lower, upper, upper - lower};
}

static u64 LoadKernelIntoMemory(
    MemoryManager* memory_manager, MultibootInfo& multiboot_info,
    const KernelModuleInfo& kernel_info
)
{
    TRACE_DEBUG("Preparing memory and loading kernel...");

    // Reserve this loader's memory temporarily
    memory_manager->MarkMemoryAreaNotFree(
        reinterpret_cast<u64>(loader_64_start), reinterpret_cast<u64>(loader_64_end)
    );

    // Find the memory map and map the kernel's virtual address space
    auto mmap_tag_res = multiboot_info.FindTag<TagMmap>();
    if (!mmap_tag_res) {
        KernelPanic("Could not find memory map tag required for kernel mapping!");
    }
    memory_manager->MapVirtualRangeUsingExternalMemoryMap<MemoryManager::WalkDirection::Descending>(
        mmap_tag_res.value(), kKernelVirtualAddressStart, kernel_info.size, 0
    );

    // Load the ELF file from the module into the newly mapped memory
    byte* module_start   = reinterpret_cast<byte*>(kernel_info.tag->mod_start);
    auto entry_point_res = Elf64::Load(module_start, 0);
    if (!entry_point_res) {
        KernelPanic("Failed to load kernel ELF module!");
    }

    return entry_point_res.value();
}

NO_RET static void TransitionToKernel(
    u64 kernel_entry_point, MemoryManager* memory_manager, const TransitionData* transition_data,
    const KernelModuleInfo& kernel_info
)
{
    TRACE_INFO("Preparing to transition to kernel...");

    // Prepare parameters for the kernel
    kernel_initial_params.kernel_start_addr   = kernel_info.lower_bound;
    kernel_initial_params.kernel_end_addr     = kernel_info.upper_bound;
    kernel_initial_params.memory_manager_addr = transition_data->memory_manager_addr;
    kernel_initial_params.multiboot_info_addr = transition_data->multiboot_info_addr;
    kernel_initial_params.multiboot_header_start_addr =
        transition_data->multiboot_header_start_addr;
    kernel_initial_params.multiboot_header_end_addr = transition_data->multiboot_header_end_addr;

    // Free this loader's memory region before jumping
    memory_manager->AddFreeMemoryRegion(
        reinterpret_cast<u64>(loader_64_start), reinterpret_cast<u64>(loader_64_end)
    );

    TRACE_INFO("Jumping to kernel at entry point: 0x%llX", kernel_entry_point);
    EnterKernel(kernel_entry_point, &kernel_initial_params);

    // This code should be unreachable
    KernelPanic("EnterKernel should not return!");
}

//==============================================================================
// Main 64-bit Loader Entry Point
//==============================================================================

extern "C" void MainLoader64(TransitionData* transition_data)
{
    InitializeLoaderEnvironment(transition_data);

    MultibootInfo multiboot_info{transition_data->multiboot_info_addr};
    auto* memory_manager = reinterpret_cast<MemoryManager*>(transition_data->memory_manager_addr);

    KernelModuleInfo kernel_info = AnalyzeKernelModule(multiboot_info);

    u64 kernel_entry_point = LoadKernelIntoMemory(memory_manager, multiboot_info, kernel_info);

    TransitionToKernel(kernel_entry_point, memory_manager, transition_data, kernel_info);
}
