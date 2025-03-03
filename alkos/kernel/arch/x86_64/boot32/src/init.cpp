#include <multiboot2.h>
#include <arch_utils.hpp>
#include <defines.hpp>
#include <elf.hpp>
#include <extensions/debug.hpp>
#include <loader_data.hpp>
#include <tags.hpp>
#include <terminal.hpp>

// External functions defined in assembly or C
extern "C" int check_cpuid();
extern "C" int check_long_mode();

extern void SetupPageTables();
extern "C" void enable_paging();
extern "C" void enable_long_mode();
extern "C" void enter_kernel(void* kernel_entry, void* multiboot_info_addr);

// Buffer for text output
char text_buffer[1024];

// External symbols defined in the linker script
extern const char multiboot_header_start[];
extern const char multiboot_header_end[];

extern const char loader_start[];
extern const char loader_end[];

// Data structure that holds information passed from the 32-bit loader to the 64-bit kernel
LoaderData loader_data;

extern "C" void PreKernelInit(uint32_t boot_loader_magic, void* multiboot_info_addr)
{
    TerminalInit();
    TRACE_INFO("In 32 bit mode");

    ////////////////////////////// Multiboot Check ///////////////////////////////
    TRACE_INFO("Checking for Multiboot2...");
    if (boot_loader_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        TRACE_ERROR("Multiboot2 check failed!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("Multiboot2 check passed!");

    ////////////////////////////// Hardware Checks ///////////////////////////////
    TRACE_INFO("Starting pre-kernel initialization");
    BlockHardwareInterrupts();

    TRACE_INFO("Checking for hardware features");

    TRACE_INFO("Checking for CPUID...");
    if (check_cpuid()) {
        TRACE_ERROR("CPUID check failed!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("CPUID check passed!");

    TRACE_INFO("Checking for long mode...");
    if (check_long_mode()) {
        TRACE_ERROR("Long mode check failed!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("Long mode check passed!");

    TRACE_INFO("Enabling hardware features...");

    //////////////////////// Setting up Paging Structures ////////////////////////
    TRACE_INFO("Setting up page tables...");
    SetupPageTables();
    TRACE_SUCCESS("Page tables setup complete!");

    ///////////////////////////// Enabling Hardware //////////////////////////////
    TRACE_INFO("Enabling long mode...");
    enable_long_mode();
    TRACE_SUCCESS("Long mode enabled!");

    TRACE_INFO("Enabling paging...");
    enable_paging();
    TRACE_SUCCESS("Paging enabled!");

    TRACE_INFO("Finished hardware features setup for 32-bit mode.");

    /////////////////////// Preparation for jump to 64 bit ///////////////////////
    TRACE_INFO("Starting 64-bit kernel...");

    TRACE_INFO("Parsing Multiboot2 tags...");

    ///////////////// Finding Kernel Module in Multiboot Struct //////////////////
    multiboot_tag_module* kernel_module = FindKernelModule(multiboot_info_addr);
    if (kernel_module == nullptr) {
        TRACE_ERROR("Failed to find kernel module!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("Found kernel module in multiboot tags!");

    /////////////////////////// Loading Kernel Module ////////////////////////////
    TRACE_INFO("Loading kernel module...");
    void* kernel_entry =
        LoadElf64Module((uint8_t*)kernel_module->mod_start, (uint8_t*)kernel_module->mod_end);
    if (kernel_entry == nullptr) {
        TRACE_ERROR("Failed to load kernel module!");
        OsHangNoInterrupts();
    }
    TRACE_SUCCESS("Kernel module loaded!");

    ///////////////////// Initializing LoaderData Structure //////////////////////
    loader_data.multiboot_info_addr         = (u32)multiboot_info_addr;
    loader_data.multiboot_header_start_addr = (u32)multiboot_header_start;
    loader_data.multiboot_header_end_addr   = (u32)multiboot_header_end;
    loader_data.loader_start_addr           = (u32)loader_start;
    loader_data.loader_end_addr             = (u32)loader_end;

    //////////////////////////// Printing LoaderData Info /////////////////////////
    // Convert addresses to hexadecimal strings
    TRACE_INFO("LoaderData Address: 0x%X", (u32)&loader_data);
    TRACE_INFO("LoaderData multiboot_info_addr: 0x%X", loader_data.multiboot_info_addr);
    TRACE_INFO(
        "LoaderData multiboot_header_start_addr: 0x%X", loader_data.multiboot_header_start_addr
    );
    TRACE_INFO("LoaderData multiboot_header_end_addr: 0x%X", loader_data.multiboot_header_end_addr);
    TRACE_INFO("LoaderData loader_start_addr: 0x%X", loader_data.loader_start_addr);
    TRACE_INFO("LoaderData loader_end_addr: 0x%X", loader_data.loader_end_addr);

    //////////////////////////// Jumping to 64-bit Kernel /////////////////////////
    TRACE_INFO("Jumping to 64-bit kernel...");
    enter_kernel(kernel_entry, &loader_data);
}
