/* Internal includes */
#include "init.hpp"
#include <libssp.h>

#include <modules/global_state.hpp>
#include <modules/timing.hpp>

void KernelInit()
{
    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the global state module */
    GlobalStateModule::Init();

    /* Initialize the timing system */
    TimingModule::Init();
}
