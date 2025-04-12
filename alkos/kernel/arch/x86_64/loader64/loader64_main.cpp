#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

/* internal includes */
#include <multiboot2/multiboot2.h>
#include <arch_utils.hpp>
#include <definitions/loader32_data.hpp>
#include <definitions/loader64_data.hpp>
#include <elf64.hpp>
#include <extensions/debug.hpp>
#include <multiboot2/extensions.hpp>
#include <terminal.hpp>
#include "loader64_kernel_constants.hpp"
#include "loader_memory_manager.hpp"

using namespace loader64;

/* external init procedures */
extern "C" void EnterKernel(u64 kernel_entry_addr, LoaderData* loader_data_kernel);
extern const char loader64_start[];
extern const char loader64_end[];
extern byte kLoaderPreAllocatedMemory[];

LoaderData loader_data;

static bool ValidateLoaderData(loader32::LoaderData* loader_data_32_64)
{
    TRACE_INFO("Checking for LoaderData...");
    if (loader_data_32_64 == nullptr) {
        TRACE_ERROR("LoaderData check failed!");
        return false;
    }
    TRACE_SUCCESS("LoaderData found passed!");

    TODO_WHEN_DEBUGGING_FRAMEWORK

    return true;
}

static multiboot::tag_module_t* FindKernelModule(u32 multiboot_info_addr)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK

    TRACE_INFO("Finding kernel module in multiboot tags...");
    auto* kernel_module = multiboot::FindTagInMultibootInfo<
        multiboot::tag_module_t, [](multiboot::tag_module_t* tag) -> bool {
            return strcmp(tag->cmdline, "kernel") == 0;
        }>(reinterpret_cast<void*>(multiboot_info_addr));
    if (kernel_module == nullptr) {
        KernelPanic("Kernel module not found in multiboot tags!");
    }
    TRACE_SUCCESS("Found kernel module in multiboot tags!");

    return kernel_module;
}

extern "C" void MainLoader64(loader32::LoaderData* loader_data_32_64)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK

    TerminalInit();
    TRACE_INFO("In 64 bit mode");

    if (!ValidateLoaderData(loader_data_32_64)) {
        KernelPanic("LoaderData check failed!");
    }

    TRACE_INFO("Jumping to 64-bit kernel...");

    auto* loader_memory_manager =
        reinterpret_cast<LoaderMemoryManager*>(loader_data_32_64->loader_memory_manager_addr);
    loader_memory_manager->MarkMemoryAreaNotFree(
        reinterpret_cast<u64>(loader64_start), reinterpret_cast<u64>(loader64_end)
    );

    auto* kernel_module = FindKernelModule(loader_data_32_64->multiboot_info_addr);

    TRACE_INFO("Getting ELF bounds...");
    auto [elf_lower_bound, elf_upper_bound] =
        elf::GetElf64ProgramBounds(reinterpret_cast<byte*>(kernel_module->mod_start));
    u64 elf_effective_size = elf_upper_bound - elf_lower_bound;
    TRACE_SUCCESS("ELF bounds obtained!");

    TRACE_INFO(
        "Mapping kernel module to upper memory starting at 0x%llX", kKernelVirtualAddressStartShared
    );
    auto* multiboot_info =
        reinterpret_cast<multiboot::header_t*>(loader_data_32_64->multiboot_info_addr);
    auto* mmap_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_mmap_t>(multiboot_info);
    loader_memory_manager
        ->MapVirtualRangeUsingExternalMemoryMap<LoaderMemoryManager::WalkDirection::Descending>(
            mmap_tag, kKernelVirtualAddressStartShared, elf_effective_size, 0
        );
    TRACE_SUCCESS("Kernel module mapped to upper memory!");

    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    loader_memory_manager->DumpPmlTables();

    byte* kernel_module_start_addr = reinterpret_cast<byte*>(kernel_module->mod_start);

    TRACE_INFO("Loading module...");
    u64 kernel_entry_point = elf::LoadElf64(kernel_module_start_addr, 0);
    if (kernel_entry_point == 0) {
        KernelPanic("Failed to load kernel module!");
    }
    TRACE_SUCCESS("Module loaded!");

    TRACE_INFO("Jumping to 64-bit kernel at 0x%llX", kernel_entry_point);

    loader_memory_manager->AddFreeMemoryRegion(
        reinterpret_cast<u64>(loader64_start), reinterpret_cast<u64>(loader64_end)
    );

    loader_data.kernel_start_addr           = elf_lower_bound;
    loader_data.kernel_end_addr             = elf_upper_bound;
    loader_data.loader_memory_manager_addr  = loader_data_32_64->loader_memory_manager_addr;
    loader_data.multiboot_info_addr         = loader_data_32_64->multiboot_info_addr;
    loader_data.multiboot_header_start_addr = loader_data_32_64->multiboot_header_start_addr;
    loader_data.multiboot_header_end_addr   = loader_data_32_64->multiboot_header_end_addr;

    EnterKernel(kernel_entry_point, &loader_data);
}
