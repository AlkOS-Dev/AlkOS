// ------------------------------
// Includes
// ------------------------------

/* main include */
#include <libssp.h>

/* external includes */
#include <stdint.h>

/* internal includes */
#include "defines.h"
#include "platform.h"
#include "todo.h"

// ------------------------------
// Stack Check Variable
// ------------------------------

TODO_RANDOM
/* TODO: random init value, should be changed by init proc */

static constexpr uintptr_t kStackChkGuard =
    UINT32_MAX == UINTPTR_MAX ? 0xe2dee396 : 0x595e9fbd94fda766;
volatile uintptr_t __stack_chk_guard = kStackChkGuard;

// ------------------------------
// libssp implementation
// ------------------------------

void __stack_chk_init() {}

TODO_BY_THE_END_OF_MILESTONE0
/* TODO: Improve messaging */

extern "C" NO_RET void __stack_chk_fail() { __platform_panic("Stack smashing detected"); }
