#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

#include <extensions/debug.hpp>

#include "cpu/utils.hpp"

#include "abi/transition_data.hpp"

#include "elf/elf64.hpp"

#include "mem/memory_manager.hpp"

#include "sys/panic.hpp"
#include "sys/terminal.hpp"

#include "multiboot2/info.hpp"
#include "multiboot2/multiboot2.h"

#include "settings.hpp"

using namespace Multiboot;

/* external init procedures */
extern "C" void EnterKernel(u64 kernel_entry_addr, KernelInitialParams* kernel_inital_params);
extern const char loader_64_start[];
extern const char loader_64_end[];
extern byte kLoaderPreAllocatedMemory[];

KernelInitialParams kernel_inital_params;

static bool ValidateTransitionData(TransitionData* transition_data)
{
    TRACE_INFO("Checking for TransitionData...");
    if (transition_data == nullptr) {
        TRACE_ERROR("TransitionData check failed!");
        return false;
    }
    TRACE_SUCCESS("TransitionData found passed!");

    TODO_WHEN_DEBUGGING_FRAMEWORK

    return true;
}

static Multiboot::TagModule* FindKernelModule(MultibootInfo multiboot_info)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK

    TRACE_INFO("Finding kernel module in multiboot tags...");
    auto* kernel_module =
        multiboot_info.FindTag<Multiboot::TagModule>([](Multiboot::TagModule* tag) -> bool {
            return strcmp(tag->cmdline, "kernel") == 0;
        });
    if (kernel_module == nullptr) {
        arch::KernelPanic("Kernel module not found in multiboot tags!");
    }
    TRACE_SUCCESS("Found kernel module in multiboot tags!");

    return kernel_module;
}

extern "C" void MainLoader64(TransitionData* transition_data)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK

    arch::TerminalInit();
    TRACE_INFO("In 64 bit mode");

    if (!ValidateTransitionData(transition_data)) {
        arch::KernelPanic("TransitionData check failed!");
    }

    TRACE_INFO("Jumping to 64-bit kernel...");

    auto* loader_memory_manager =
        reinterpret_cast<LoaderMemoryManager*>(transition_data->loader_memory_manager_addr);
    loader_memory_manager->MarkMemoryAreaNotFree(
        reinterpret_cast<u64>(loader_64_start), reinterpret_cast<u64>(loader_64_end)
    );

    auto* kernel_module = FindKernelModule(transition_data->multiboot_info_addr);

    TRACE_INFO("Getting ELF bounds...");
    auto [elf_lower_bound, elf_upper_bound] =
        elf::GetElf64ProgramBounds(reinterpret_cast<byte*>(kernel_module->mod_start));
    u64 elf_effective_size = elf_upper_bound - elf_lower_bound;
    TRACE_SUCCESS("ELF bounds obtained!");

    TRACE_INFO(
        "Mapping kernel module to upper memory starting at 0x%llX", kKernelVirtualAddressStart
    );
    MultibootInfo multiboot_info{transition_data->multiboot_info_addr};
    auto* mmap_tag = multiboot_info.FindTag<Multiboot::TagMmap>();
    loader_memory_manager
        ->MapVirtualRangeUsingExternalMemoryMap<LoaderMemoryManager::WalkDirection::Descending>(
            mmap_tag, kKernelVirtualAddressStart, elf_effective_size, 0
        );
    TRACE_SUCCESS("Kernel module mapped to upper memory!");

    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    loader_memory_manager->DumpPmlTables();

    byte* kernel_module_start_addr = reinterpret_cast<byte*>(kernel_module->mod_start);

    TRACE_INFO("Loading module...");
    u64 kernel_entry_point = elf::LoadElf64(kernel_module_start_addr, 0);
    if (kernel_entry_point == 0) {
        arch::KernelPanic("Failed to load kernel module!");
    }
    TRACE_SUCCESS("Module loaded!");

    TRACE_INFO("Jumping to 64-bit kernel at 0x%llX", kernel_entry_point);

    loader_memory_manager->AddFreeMemoryRegion(
        reinterpret_cast<u64>(loader_64_start), reinterpret_cast<u64>(loader_64_end)
    );

    kernel_inital_params.kernel_start_addr           = elf_lower_bound;
    kernel_inital_params.kernel_end_addr             = elf_upper_bound;
    kernel_inital_params.loader_memory_manager_addr  = transition_data->loader_memory_manager_addr;
    kernel_inital_params.multiboot_info_addr         = transition_data->multiboot_info_addr;
    kernel_inital_params.multiboot_header_start_addr = transition_data->multiboot_header_start_addr;
    kernel_inital_params.multiboot_header_end_addr   = transition_data->multiboot_header_end_addr;

    EnterKernel(kernel_entry_point, &kernel_inital_params);
}
