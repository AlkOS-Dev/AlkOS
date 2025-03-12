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
#include <elf/elf64.hpp>
#include <extensions/debug.hpp>
#include <multiboot2/extensions.hpp>
#include <terminal.hpp>
#include "definitions/page_buffer.hpp"
#include "loader64_kernel_constants.hpp"
#include "loader_memory_manager/loader_memory_manager.hpp"

using namespace loader64;

/* external init procedures */
extern "C" void EnterKernel(u64 kernel_entry_addr, LoaderData* loader_data_kernel);

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

static u64 GetTotalMemoryBytes(multiboot::tag_mmap_t* mmap_tag)
{
    u64 total_memory_bytes = 0;
    multiboot::WalkMemoryMap(mmap_tag, [&](multiboot::memory_map_t* mmap_entry) FORCE_INLINE_L {
        if (mmap_entry->type == multiboot::memory_map_t::kMemoryAvailable) {
            total_memory_bytes += mmap_entry->len;
        }
    });

    return total_memory_bytes;
}

static PageBufferParams_t CreatePageBuffer(
    u64 address_to_create_at, u64 memory_size_to_handle_bytes, u64 page_size,
    LoaderMemoryManager* loader_memory_manager
)
{
    PageBufferParams_t page_buffer_params;
    page_buffer_params.buffer_addr            = address_to_create_at;
    page_buffer_params.total_size_num_pages   = memory_size_to_handle_bytes / page_size;
    page_buffer_params.current_size_num_pages = 0;

    page_buffer_params.buffer_addr =
        AlignUp(page_buffer_params.buffer_addr, loader64::kPhysicalPageSize);
    loader_memory_manager->MapVirtualRangeUsingInternalMemoryMap(
        page_buffer_params.buffer_addr, page_buffer_params.total_size_num_pages * sizeof(u64)
    );

    return page_buffer_params;
}

static void FillPageBuffer(
    PageBufferParams_t& page_buffer_params, LoaderMemoryManager* loader_memory_manager
)
{
    loader_memory_manager->WalkFreeMemoryRegions([&](FreeMemoryRegion_t& region) {
        TRACE_INFO(
            "Region: 0x%llX-0x%llX, length: %llu kB", region.addr, region.addr + region.length,
            region.length >> 10
        );
        u64* buffer           = reinterpret_cast<u64*>(page_buffer_params.buffer_addr);
        u64 region_start_addr = AlignUp(region.addr, loader64::kPhysicalPageSize);
        for (u64 i = region_start_addr; i < region_start_addr + region.length;
             i += loader64::kPhysicalPageSize) {
            R_ASSERT(
                page_buffer_params.current_size_num_pages < page_buffer_params.total_size_num_pages
            );
            buffer[page_buffer_params.current_size_num_pages++] = i;
        }
    });
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

extern "C" void MainLoader64(loader32::LoaderData* loader_data_32_64)
{
    TerminalInit();
    TRACE_INFO("In 64 bit mode");

    if (!ValidateLoaderData(loader_data_32_64)) {
        KernelPanic("LoaderData check failed!");
    }

    TRACE_INFO("Jumping to 64-bit kernel...");

    auto* loader_memory_manager =
        reinterpret_cast<LoaderMemoryManager*>(loader_data_32_64->loader_memory_manager_addr);

    auto* kernel_module = FindKernelModule(loader_data_32_64->multiboot_info_addr);

    TRACE_INFO("Getting ELF bounds...");
    auto [elf_lower_bound, elf_upper_bound] =
        elf::GetElf64ProgramBounds(reinterpret_cast<byte*>(kernel_module->mod_start));
    u64 elf_effective_size = elf_upper_bound - elf_lower_bound;
    TRACE_SUCCESS("ELF bounds obtained!");

    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    TRACE_INFO(
    //        "ELF bounds: 0x%llX-0x%llX, size %llu Kb", elf_lower_bound, elf_upper_bound,
    //        elf_effective_size >> 10
    //    );

    TRACE_INFO(
        "Mapping kernel module to upper memory starting at 0x%llX", kKernelVirtualAddressStartShared
    );
    loader_memory_manager->MapVirtualRangeUsingInternalMemoryMap(
        kKernelVirtualAddressStartShared, elf_effective_size, 0
    );

    TRACE_SUCCESS("Kernel module mapped to upper memory!");

    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    loader_memory_manager->DumpPmlTables();

    auto* mmap_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_mmap_t>(
        reinterpret_cast<void*>(loader_data_32_64->multiboot_info_addr)
    );

    u64 total_memory_bytes = GetTotalMemoryBytes(mmap_tag);

    TODO_WHEN_DEBUGGING_FRAMEWORK
    TRACE_INFO("Total available memory: %llu MB", total_memory_bytes >> 20);

    byte* kernel_module_start_addr = reinterpret_cast<byte*>(kernel_module->mod_start);

    TRACE_INFO("Loading module...");
    u64 kernel_entry_point = elf::LoadElf64(kernel_module_start_addr, 0);
    if (kernel_entry_point == 0) {
        KernelPanic("Failed to load kernel module!");
    }
    TRACE_SUCCESS("Module loaded!");

    TRACE_INFO("Loading module at 0x%llX", kernel_entry_point);

    TRACE_INFO("Jumping to 64-bit kernel...");

    EnterKernel(kernel_entry_point, &loader_data);
}
