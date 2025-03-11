/* Internal includes */
#include "init.hpp"
#include <libssp.h>
#include <modules/timing.hpp>

void KernelInit()
{
    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the timing system */
    TimingModule::Init();
}
