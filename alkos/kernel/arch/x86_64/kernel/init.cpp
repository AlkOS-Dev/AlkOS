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
#include <interrupts/idt.hpp>
#include <terminal.hpp>

/* external init procedures */
extern "C" void EnableOSXSave();
extern "C" void EnableSSE();
extern "C" void EnableAVX();
extern "C" void EnterKernel(u64 kernel_entry_addr);

extern "C" void PreKernelInit(loader64::LoaderData* loader_data)
{
    TerminalInit();
    TRACE_INFO("In 64 bit mode");

    TRACE_INFO("Checking for LoaderData...");
    TODO_WHEN_DEBUGGING_FRAMEWORK
    TRACE_INFO("LoaderData Address: 0x%X", loader_data);
    TRACE_INFO("LoaderData buffer_addr: 0x%X", loader_data->page_buffer_params.buffer_addr);
    TRACE_INFO(
        "LoaderData total_size_num_pages: %llu",
        loader_data->page_buffer_params.total_size_num_pages
    );
    // TODO: For now, loader_data is garbage, it's not passed from the 64-bit loader
    //    if (loader_data == nullptr) {
    //        TRACE_ERROR("LoaderData check failed!");
    //        OsHangNoInterrupts();
    //    }
    TRACE_SUCCESS("LoaderData found!");
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
}
