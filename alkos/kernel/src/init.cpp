/* Internal includes */
#include "init.hpp"
#include <libssp.h>

#include <acpi/acpi.hpp>
#include <modules/global_state.hpp>
#include <modules/hardware.hpp>
#include <modules/timing.hpp>

void KernelInit()
{
    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the global state module */
    GlobalStateModule::Init();

    /* Allow hardware to fully initialise interrupt system */
    HardwareModule::Get().GetInterrupts().Initialise();

    /* Initialize the timing system */
    TimingModule::Init();

    /* Initialize ACPI */
    ACPI::Init();
}
