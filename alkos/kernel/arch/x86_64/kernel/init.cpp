#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

/* internal includes */
#include <multiboot2/multiboot2.h>
#include <arch_utils.hpp>
#include <definitions/loader64_data.hpp>
#include <drivers/pic8259/pic8259.hpp>
#include <extensions/debug.hpp>
#include <extensions/internal/formats.hpp>
#include <interrupts/idt.hpp>
#include <loader_memory_manager.hpp>
#include <multiboot2/extensions.hpp>
#include <terminal.hpp>
#include "memory_management/physical_memory_manager.hpp"

/* external init procedures */
extern "C" void EnableOSXSave();
extern "C" void EnableSSE();
extern "C" void EnableAVX();
extern "C" void EnterKernel(u64 kernel_entry_addr);

loader64::LoaderData* kLoaderData;

static memory::PhysicalMemoryManager::PageBufferInfo_t CreatePageBuffer(
    loader64::LoaderData* loader_data, LoaderMemoryManager* loader_memory_manager
)
{
    TRACE_INFO("Creating page buffer...");
    memory::PhysicalMemoryManager::PageBufferInfo_t buffer_info{};
    auto* mmap_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_mmap_t>(
        reinterpret_cast<void*>(loader_data->multiboot_info_addr)
    );

    u64 total_memory_bytes = 0;
    multiboot::WalkMemoryMap(mmap_tag, [&total_memory_bytes](multiboot::memory_map_t* entry) {
        if (entry->type == multiboot::mmap_entry_t::kMemoryAvailable) {
            total_memory_bytes += entry->len;
        }
    });

    u64 pages_required = total_memory_bytes / memory::PhysicalMemoryManager::kPageSize + 1;

    buffer_info.start_addr = AlignUp(
        loader_data->kernel_end_addr + memory::PhysicalMemoryManager::kPageSize,
        memory::PhysicalMemoryManager::kPageSize
    );
    buffer_info.size_bytes = pages_required * sizeof(u64);

    loader_memory_manager
        ->MapVirtualRangeUsingExternalMemoryMap<LoaderMemoryManager::WalkDirection::Descending>(
            mmap_tag, buffer_info.start_addr, buffer_info.size_bytes
        );
    TRACE_INFO("Total memory bytes: %sB", FormatMetricUint(total_memory_bytes));
    TRACE_INFO("Pages required: %sB", FormatMetricUint(pages_required));

    loader_data->multiboot_header_end_addr = buffer_info.start_addr + buffer_info.size_bytes;

    TRACE_SUCCESS("Page buffer created!");
    return buffer_info;
}

extern "C" void PreKernelInit(loader64::LoaderData* loader_data)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK

    arch::TerminalInit();
    TRACE_INFO("In 64 bit mode");

    TRACE_INFO("Checking for LoaderData...");
    if (loader_data == nullptr) {
        TRACE_ERROR("LoaderData check failed!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("LoaderData found!");
    TODO_WHEN_DEBUGGING_FRAMEWORK
    TRACE_INFO("Starting pre-kernel initialization");

    TRACE_INFO("Starting to setup CPU features");
    BlockHardwareInterrupts();

    /* NOTE: sequence is important */
    TRACE_INFO("Setting up OS XSAVE...");
    EnableOSXSave();
    TRACE_SUCCESS("OS XSAVE setup complete!");

    TRACE_INFO("Setting up SSE...");
    EnableSSE();
    TRACE_SUCCESS("SSE setup complete!");

    TRACE_INFO("Setting up AVX...");
    EnableAVX();
    TRACE_SUCCESS("AVX setup complete!");

    TRACE_INFO("Setting up PIC units...");
    InitPic8259(kIrq1Offset, kIrq2Offset);
    TRACE_SUCCESS("PIC units setup complete!");

    TRACE_INFO("Setting up IDT...");
    IdtInit();
    TRACE_SUCCESS("IDT setup complete!");

    EnableHardwareInterrupts();
    TRACE_INFO("Finished cpu features setup.");

    auto* loader_memory_manager =
        reinterpret_cast<LoaderMemoryManager*>(loader_data->loader_memory_manager_addr);
    loader_memory_manager->MarkMemoryAreaNotFree(
        loader_data->kernel_start_addr, loader_data->kernel_end_addr
    );

    memory::PhysicalMemoryManager::PageBufferInfo_t page_buffer_info =
        CreatePageBuffer(loader_data, loader_memory_manager);
    PhysicalMemoryManager::Init(page_buffer_info);

    auto* multiboot_info = reinterpret_cast<multiboot::header_t*>(loader_data->multiboot_info_addr);
    auto* mmap_tag       = multiboot::FindTagInMultibootInfo<multiboot::tag_mmap_t>(multiboot_info);
    PhysicalMemoryManager::Get().PopulatePageBuffer(mmap_tag);

    kLoaderData = loader_data;
}
