/* Internal includes */
#include "init.hpp"
#include <libssp.h>

#include <acpi/acpi.hpp>
#include <modules/global_state.hpp>
#include <modules/hardware.hpp>
#include <modules/memory.hpp>
#include <modules/timing.hpp>

void KernelInit()
{
    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the global state module */
    GlobalStateModule::Init();

    MemoryModule::Get().GetPmm().Init();
    MemoryModule::Get().GetVmm().Init();

    // TODO: DISABLING ACPI AND INTERRUPTS UNTIL
    // PROPER HANDLING OF MEMORY AND MULTIBOOT IS DONE
    //
    // /* Initialize ACPI */
    // HardwareModule::Get().GetACPIController().Init();

    // /* Extract all necessary data from ACPI tables */
    // HardwareModule::Get().GetACPIController().ParseTables();

    // /* Allow hardware to fully initialise interrupt system */
    // HardwareModule::Get().GetInterrupts().Initialise();

    /* Initialize the timing system */
    TimingModule::Init();
}
