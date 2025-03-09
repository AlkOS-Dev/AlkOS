/* external incldue */
#include <stddef.h>

/* internal include */
#include <defines.h>

#ifdef __ALKOS_LIBK__
#include <panic.hpp>
#endif

// -------------------------------------------
// Static initializer expected functions
// -------------------------------------------

/**
 * @todo: Implement the following functions using xchg, cmpxchg, or similar atomic instructions,
 * when races are possible.
 */

__extension__ typedef int __guard __attribute__((mode(__DI__)));

extern "C" int __cxa_guard_acquire(__guard *)
{
    /* TODO */
    return 1;
}

extern "C" void __cxa_guard_release(__guard *) { /* TODO */ }

extern "C" void __cxa_guard_abort(__guard *)
{
#if __STDC_HOSTED__

    /* TODO */

#else

    KernelPanic("Failed to initialize static object");

#endif
}

extern "C" void __cxa_atexit(void) {}

// -----------------------------------
// New and delete implementation
// -----------------------------------

void *operator new(size_t)
{
    /* TODO */
    return reinterpret_cast<void *>(0xdeadc0de);
}

void operator delete(void *) { /* TODO */ }

void operator delete(void *, size_t) { /* TODO */ }

void *operator new[](size_t)
{
    /* TODO */
    return reinterpret_cast<void *>(0xdeadc0de);
}

void operator delete[](void *) { /* TODO */ }

void operator delete[](void *, size_t) { /* TODO */ }
