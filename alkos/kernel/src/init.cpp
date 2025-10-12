#include <libssp.h>

/* Internal includes */
#include "acpi/acpi.hpp"
#include "hal/kernel_arguments.hpp"
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "modules/memory.hpp"
#include "modules/timing.hpp"

/* GCC CXX provided function initializing global constructors */
extern "C" void _init();

void KernelInit(const hal::KernelArguments& args)
{
    /* GCC CXX provided function initializing global constructors */
    _init();

    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the global state module */
    GlobalStateModule::Init();

    MemoryModule::Get().GetPmm().Init();
    MemoryModule::Get().GetVmm().Init();

    TODO_MMU_MINIMAL
    // /* Initialize ACPI */
    // HardwareModule::Get().GetACPIController().Init();

    // /* Extract all necessary data from ACPI tables */
    // HardwareModule::Get().GetACPIController().ParseTables();

    // /* Allow hardware to fully initialise interrupt system */
    // HardwareModule::Get().GetInterrupts().Initialise();

    /* Initialize the timing system */
    // TimingModule::Init();
}
