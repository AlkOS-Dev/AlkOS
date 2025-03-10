#include <init.hpp>

/* Internal includes */
#include <libssp.h>

#include <modules/global_state.hpp>
#include <modules/timing.hpp>

/**
 * @brief Performs all high-level initialization not related to architecture
 * @note  MUST ALWAYS be invoked after the PreKernelInit function
 */
void KernelInit()
{
    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the global state module */
    GlobalStateModule::Init();

    /* Initialize the timing system */
    TimingModule::Init();
}
