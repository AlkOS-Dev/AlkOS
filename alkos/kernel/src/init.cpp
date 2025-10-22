#include <libssp.h>

/* Internal includes */
#include "acpi/acpi.hpp"
#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "hal/terminal.hpp"
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "modules/memory.hpp"
#include "modules/timing.hpp"
#include "trace.hpp"

/* GCC CXX provided function initializing global constructors */
extern "C" void _init();

void KernelInit(const hal::RawBootArguments &raw_args)
{
    hal::TerminalInit();
    hal::ArchInit(raw_args);

    BootArguments args = SanitizeBootArgs(raw_args);
    KernelTraceInfo("Sanitized boot arguments received by kernel:");
    KernelTrace(
        "  Boot Arguments:\n"
        "    kernel_start:       0x%p\n"
        "    kernel_end:         0x%p\n"
        "    root_page_table:    0x%p\n"
        "    mem_bitmap:         0x%p\n"
        "    total_page_frames:  %zu\n"
        "    multiboot_info:     0x%p\n",
        args.kernel_start, args.kernel_end, args.root_page_table, args.mem_bitmap,
        args.total_page_frames, args.multiboot_info
    );

    MemoryModule::Init(args);

    /* GCC CXX provided function initializing global constructors */
    _init();

    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the global state module */
    GlobalStateModule::Init();

    /* Initialize ACPI */
    // HardwareModule::Get().GetACPIController().Init(args);

    /* Extract all necessary data from ACPI tables */
    // HardwareModule::Get().GetACPIController().ParseTables();

    /* Allow hardware to fully initialise interrupt system */
    // HardwareModule::Get().GetInterrupts().Init();

    /* Initialize the timing system */
    // TimingModule::Init();
}
