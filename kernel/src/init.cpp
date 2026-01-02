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
#include "modules/window.hpp"
#include "trace_framework.hpp"

/* GCC CXX provided function initializing global constructors */
extern "C" void _init();

void KernelInit(const hal::RawBootArguments &raw_args)
{
    hal::TerminalInit();
    hal::ArchInit(raw_args);

    BootArguments args = SanitizeBootArgs(raw_args);

    MemoryModule::Init(args);
    MemoryModule::Get().RegisterKernelVMAreas(args);

    /* GCC CXX provided function initializing global constructors */
    _init();

    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the global state module */
    GlobalStateModule::Init();

    /* Early Initialize ACPI subsystem for table parsing */
    HardwareModule::Get().GetACPIController().EarlyInit(args);

    /* Extract all necessary data from ACPI tables */
    HardwareModule::Get().GetACPIController().ParseTables();

    /* Allow hardware to fully initialize interrupt system */
    HardwareModule::Get().GetInterrupts().Init();

    TimingModule::Init();

    /* Register interrupt handlers */
    HardwareModule::Get().RegisterInterruptHandlers();

    /* Fully Initialize ACPI subsystem */
    // HardwareModule::Get().GetACPIController().Init();

    VfsModule::Init(args);

    SchedulingModule::Init();
    SchedulingModule::Get().GetTaskMgr().InitializeMultitasking();

    HardwareModule::Get().GetCoresController().BootUpAllCores();

    VideoModule::Init(args, MemoryModule::Get().GetHeap());
    WindowModule::Init();
}
