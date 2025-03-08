#include <loader64_main.hpp>

/* Internal includes */
#include <libssp.h>
#include <timing.hpp>

/**
 * @brief Performs all high-level initialization not related to architecture
 * @note  MUST ALWAYS be invoked after the PreKernelInit function
 */
void KernelInit()
{
    /* Initialize the stack protection */
    __stack_chk_init();

    /* Initialize the timing system */
    InitTiming();
}
