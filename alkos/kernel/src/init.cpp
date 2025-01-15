#include <init.hpp>

/* Internal includes */
#include <libssp.h>

/**
 * @brief Performs all high-level initialization not related to architecture
 * @note  MUST ALWAYS be invoked after the PreKernelInit function
 */
void KernelInit() { __stack_chk_init(); }
