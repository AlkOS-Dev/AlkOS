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
#include <multiboot2/extensions.hpp>
#include <terminal.hpp>
#include "loader64_kernel_constants.hpp"
#include "loader_memory_manager/loader_memory_manager.hpp"

/* external init procedures */
extern "C" void EnterKernel(u64 kernel_entry_addr);

static bool ValidateLoaderData(LoaderData_32_64_Pass* loader_data)
{
    TRACE_INFO("Checking for LoaderData...");
    if (loader_data == nullptr) {
        TRACE_ERROR("LoaderData check failed!");
        return false;
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

    return true;
}

static multiboot::tag_new_acpi_t* FindAcpiTag(u32 multiboot_info_addr)
{
    TRACE_INFO("Finding ACPI tag in multiboot tags...");
    auto* new_acpi_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_new_acpi_t>(
        reinterpret_cast<void*>(multiboot_info_addr)
    );
    if (new_acpi_tag == nullptr) {
        TRACE_WARNING("ACPI2.0 tag not found in multiboot tags, trying ACPI1.0 tag...");
        auto* old_acpi_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_old_acpi_t>(
            reinterpret_cast<void*>(multiboot_info_addr)
        );
        if (old_acpi_tag == nullptr) {
            KernelPanic("ACPI1.0 tag not found in multiboot tags!");
        }
        new_acpi_tag = reinterpret_cast<multiboot::tag_new_acpi_t*>(old_acpi_tag);
    }
    TRACE_SUCCESS("Found ACPI tag in multiboot tags!");

    return new_acpi_tag;
}

static multiboot::tag_module_t* FindKernelModule(u32 multiboot_info_addr)
{
    TRACE_INFO("Finding kernel module in multiboot tags...");
    auto* kernel_module = multiboot::FindTagInMultibootInfo<
        multiboot::tag_module_t, [](multiboot::tag_module_t* tag) -> bool {
            TODO_WHEN_DEBUGGING_FRAMEWORK
            //            TRACE_INFO("Checking tag with cmdline: %s", tag->cmdline);
            return strcmp(tag->cmdline, "kernel") == 0;
        }>(reinterpret_cast<void*>(multiboot_info_addr));
    if (kernel_module == nullptr) {
        KernelPanic("Kernel module not found in multiboot tags!");
    }
    TRACE_SUCCESS("Found kernel module in multiboot tags!");

    return kernel_module;
}

extern "C" void MainLoader64(LoaderData_32_64_Pass* loader_data)
{
    TerminalInit();
    TRACE_INFO("In 64 bit mode");

    if (!ValidateLoaderData(loader_data)) {
        KernelPanic("LoaderData check failed!");
    }

    TRACE_INFO("Jumping to 64-bit kernel...");

    auto* loader_memory_manager =
        reinterpret_cast<LoaderMemoryManager*>(loader_data->loader_memory_manager_addr);

    auto* kernel_module = FindKernelModule(loader_data->multiboot_info_addr);

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

    TRACE_INFO(
        "Mapping kernel module to upper memory starting at 0x%llX", kKernelVirtualAddressStartShared
    );
    loader_memory_manager->MapVirtualRangeUsingInternalMemoryMap(
        kKernelVirtualAddressStartShared, elf_effective_size, 0
    );
    TRACE_SUCCESS("Kernel module mapped to upper memory!");

    TRACE_INFO("Mapping ACPI tables...");
    auto* acpi_tag = FindAcpiTag(loader_data->multiboot_info_addr);
    loader_memory_manager->MapVirtualRangeUsingInternalMemoryMap(
        kACPIRsdpAddrShared, acpi_tag->size - sizeof(multiboot::tag_new_acpi_t), 0
    );
    TRACE_SUCCESS("ACPI tables mapped!");

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
