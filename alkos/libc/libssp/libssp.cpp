// ------------------------------
// Includes
// ------------------------------

/* main include */
#include <libssp.hpp>

/* external includes */
#include <stdint.h>

/* internal includes */
#include <defines.hpp>

#ifdef __ALKOS_KERNEL__
#include "../../kernel/abi/panic.hpp"
#endif  // __ALKOS_KERNEL__

#if __STDC_HOSTED__
#error "NOT IMPLEMENTED"
#endif  // __STDC_HOSTED__

// ------------------------------
// Stack Check Variable
// ------------------------------

/* random init value, should be changed by init proc */
static constexpr uintptr_t kStackChkGuard =
    UINT32_MAX == UINTPTR_MAX ? 0xe2dee396 : 0x595e9fbd94fda766;
volatile uintptr_t __stack_chk_guard = kStackChkGuard;

// ------------------------------
// Host implementation
// ------------------------------

#if __STDC_HOSTED__

static void __stack_chk_init_hosted() {}

static __attribute__((noreturn)) void __stack_chk_fail_hosted() {}

#endif  // __STDC_HOSTED__

// ------------------------------
// Kernel implementation
// ------------------------------

#ifdef __ALKOS_KERNEL__

/**
 * @todo Implement this when random number generator is implemented
 */
static void __stack_chk_init_kernel() {}

/**
 * @todo Add some debug message about stack in future
 */
API_CALL static __attribute__((noreturn)) void __stack_chk_fail_kernel()
{
    KernelPanic("Stack smashing detected");
}

#endif  // __ALKOS_KERNEL__

// ------------------------------
// libssp implementation
// ------------------------------

void __stack_chk_init()
{
#if __STDC_HOSTED__

    __stack_chk_init_hosted();

#else

    __stack_chk_init_kernel();

#endif
}

extern "C" __attribute__((noreturn)) void __stack_chk_fail()
{
#if __STDC_HOSTED__

    __stack_chk_fail_hosted();

#else

    __stack_chk_fail_kernel();

#endif
}
