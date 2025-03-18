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
#include <loader_memory_manager.hpp>
#include <terminal.hpp>

/* external init procedures */
extern "C" void EnableOSXSave();
extern "C" void EnableSSE();
extern "C" void EnableAVX();
extern "C" void EnterKernel(u64 kernel_entry_addr);

static void InitializePhysicalMemoryManager(loader64::LoaderData* loader_data)
{
    auto* loader_memory_manager =
        reinterpret_cast<LoaderMemoryManager*>(loader_data->loader_memory_manager_addr);
}

extern "C" void PreKernelInit(loader64::LoaderData* loader_data)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK

    TerminalInit();
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
}
