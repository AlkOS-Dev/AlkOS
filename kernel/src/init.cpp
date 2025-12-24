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
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"
#include "modules/vfs.hpp"
#include "modules/video.hpp"
#include "trace_framework.hpp"

/* GCC CXX provided function initializing global constructors */
extern "C" void _init();

void KernelInit(const hal::RawBootArguments &raw_args)
{
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
    HardwareModule::Get().GetACPIController().Init(args);

    /* Extract all necessary data from ACPI tables */
    HardwareModule::Get().GetACPIController().ParseTables();

    /* Allow hardware to fully initialise interrupt system */
    HardwareModule::Get().GetInterrupts().Init();

    /* Initialize the timing system */
    TimingModule::Init();

    VfsModule::Init(args);

    VideoModule::Init(args, MemoryModule::Get().GetHeap());

    // Register Interrupts
    MemoryModule::Get().RegisterPageFault(HardwareModule::Get());
    HardwareModule::Get().RegisterInterruptHandlers();

    SchedulingModule::Init();
    SchedulingModule::Get().GetTaskMgr().InitializeMultitasking();
}
