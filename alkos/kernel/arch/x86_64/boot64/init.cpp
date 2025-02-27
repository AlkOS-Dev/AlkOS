#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

/* internal includes */
#include <multiboot2.h>
#include <arch_utils.hpp>
#include <debug.hpp>
#include <drivers/pic8259/pic8259.hpp>
#include <init.hpp>
#include <interrupts/idt.hpp>
#include <loader_data.hpp>
#include <multiboot2_extensions.hpp>
#include <terminal.hpp>

/* external init procedures */
extern "C" void enable_osxsave();

extern "C" void enable_sse();

extern "C" void enable_avx();

extern "C" void PreKernelInit(LoaderData *loader_data)
{
    TerminalInit();
    TRACE_INFO("In 64 bit mode");

    TRACE_INFO("Checking for LoaderData...");
    TRACE_INFO("LoaderData Address: 0x%X", loader_data);
    if (loader_data == nullptr) {
        KernelPanic("LoaderData not found!");
    }
    TRACE_INFO("LoaderData multiboot_info_addr: 0x%X", loader_data->multiboot_info_addr);
    TRACE_INFO(
        "LoaderData multiboot_header_start_addr: 0x%X", loader_data->multiboot_header_start_addr
    );
    TRACE_INFO(
        "LoaderData multiboot_header_end_addr: 0x%X", loader_data->multiboot_header_end_addr
    );
    TRACE_INFO("LoaderData loader_start_addr: 0x%X", loader_data->loader_start_addr);
    TRACE_INFO("LoaderData loader_end_addr: 0x%X", loader_data->loader_end_addr);

    TRACE_INFO("Starting pre-kernel initialization");

    TRACE_INFO("Starting to setup CPU features");
    BlockHardwareInterrupts();

    /* NOTE: sequence is important */
    TRACE_INFO("Setting up OS XSAVE...");
    enable_osxsave();
    TRACE_SUCCESS("OS XSAVE setup complete!");

    TRACE_INFO("Setting up SSE...");
    enable_sse();
    TRACE_SUCCESS("SSE setup complete!");

    TRACE_INFO("Setting up AVX...");
    enable_avx();
    TRACE_SUCCESS("AVX setup complete!");

    TRACE_INFO("Setting up PIC units...");
    InitPic8259(kIrq1Offset, kIrq2Offset);
    TRACE_SUCCESS("PIC units setup complete!");

    TRACE_INFO("Setting up IDT...");
    IdtInit();
    TRACE_SUCCESS("IDT setup complete!");

    EnableHardwareInterrupts();
    TRACE_INFO("Finished cpu features setup.");

    TRACE_INFO("Mapping physical memory...");
    auto *mmap_tag = reinterpret_cast<multiboot_tag_mmap *>(FindTagInMultibootInfo(
        reinterpret_cast<void *>(loader_data->multiboot_info_addr), MULTIBOOT_TAG_TYPE_MMAP
    ));
    if (mmap_tag == nullptr) {
        KernelPanic("Memory map tag not found!");
    }
    TRACE_INFO("Memory map tag found!");

    uint64_t total_memory_size = 0;
    WalkMemoryMap(mmap_tag, [&total_memory_size](multiboot_memory_map_t *mmap_entry) {
        if (mmap_entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            total_memory_size += mmap_entry->len;
        }
    });

    TRACE_INFO("Total memory size: %d MB", total_memory_size / 1024 / 1024);

    TRACE_INFO("Pre-kernel initialization finished.");
}
