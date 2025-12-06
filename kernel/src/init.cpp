#include <libssp.h>

/* Internal includes */
#include "acpi/acpi.hpp"
#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "hal/debug.hpp"
#include "hal/terminal.hpp"
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "modules/memory.hpp"
#include "modules/timing.hpp"
#include "modules/video.hpp"
#include "trace_framework.hpp"

/* GCC CXX provided function initializing global constructors */
extern "C" void _init();

static void test() { hal::DebugStack(); }

void KernelInit(const hal::RawBootArguments &raw_args)
{
    hal::DebugStack();
    test();

    hal::TerminalInit();
    hal::ArchInit(raw_args);

    BootArguments args = SanitizeBootArgs(raw_args);

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

    /* Setup core local data */

    /* Initialize the timing system */
    // TimingModule::Init();

    MemoryModule::Get().RegisterPageFault(HardwareModule::Get());

    VideoModule::Init(args);
}
