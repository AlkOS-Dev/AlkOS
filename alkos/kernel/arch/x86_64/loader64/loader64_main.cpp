#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

/* internal includes */
#include <multiboot2/multiboot2.h>
#include <arch_utils.hpp>
#include <elf/elf64.hpp>
#include <extensions/debug.hpp>
#include <loader_data.hpp>
#include <loader_memory_manager/loader_memory_manager.hpp>
#include <multiboot2/extensions.hpp>
#include <terminal.hpp>

/* external init procedures */
extern "C" void EnterKernel(u64 kernel_entry_addr);

extern "C" void MainLoader64(LoaderData_32_64_Pass* loader_data)
{
    TerminalInit();
    TRACE_INFO("In 64 bit mode");

    TRACE_INFO("Checking for LoaderData...");
    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    TRACE_INFO("LoaderData Address: 0x%X", loader_data);
    if (loader_data == nullptr) {
        TRACE_ERROR("LoaderData check failed!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("LoaderData found passed!");
    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    TRACE_INFO("LoaderData multiboot_info_addr: 0x%X", loader_data->multiboot_info_addr);
    //    TRACE_INFO(
    //        "LoaderData multiboot_header_start_addr: 0x%X",
    //        loader_data->multiboot_header_start_addr
    //    );
    //    TRACE_INFO(
    //        "LoaderData multiboot_header_end_addr: 0x%X", loader_data->multiboot_header_end_addr
    //    );
    //    TRACE_INFO("LoaderData loader_start_addr: 0x%X", loader_data->loader_start_addr);
    //    TRACE_INFO("LoaderData loader_end_addr: 0x%X", loader_data->loader_end_addr);
    //
    TRACE_INFO("Starting pre-kernel initialization");

    TRACE_INFO("Jumping to 64-bit kernel...");

    TRACE_INFO("Finding kernel module in multiboot tags...");
    auto* kernel_module = multiboot::FindTagInMultibootInfo<
        multiboot::tag_module_t, [](multiboot::tag_module_t* tag) -> bool {
            TODO_WHEN_DEBUGGING_FRAMEWORK
            //            TRACE_INFO("Checking tag with cmdline: %s", tag->cmdline);
            return strcmp(tag->cmdline, "kernel") == 0;
        }>(reinterpret_cast<void*>(loader_data->multiboot_info_addr));
    if (kernel_module == nullptr) {
        TRACE_ERROR("Kernel module not found in multiboot tags!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("Found kernel module in multiboot tags!");

    TRACE_INFO("Getting ELF bounds...");
    auto [elf_lower_bound, elf_upper_bound] =
        elf::GetElf64ProgramBounds(reinterpret_cast<byte*>(kernel_module->mod_start));
    u64 elf_effective_size = elf_upper_bound - elf_lower_bound;
    TRACE_SUCCESS("ELF bounds obtained!");

    TODO_WHEN_DEBUGGING_FRAMEWORK
    TRACE_INFO(
        "ELF bounds: 0x%llX-0x%llX, size %llu Kb", elf_lower_bound, elf_upper_bound,
        elf_effective_size >> 10
    );

    auto* loader_memory_manager =
        reinterpret_cast<LoaderMemoryManager*>(loader_data->loader_memory_manager_addr);
    static constexpr u64 kUpperCanonicalAddress = 0xFFFFFFFF80000000;

    TRACE_INFO("Mapping kernel module to upper memory starting at 0x%llX", kUpperCanonicalAddress);
    loader_memory_manager->MapVirtualRangeUsingInternalMemoryMap(
        kUpperCanonicalAddress, elf_effective_size, 0
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

    TRACE_INFO("Loading module at 0x%llX", kernel_entry_point);

    TRACE_INFO("Jumping to 64-bit kernel...");

    // TODO: Make this pass the loader data - (When implementing physical memory manager)
    EnterKernel(kernel_entry_point);
}
