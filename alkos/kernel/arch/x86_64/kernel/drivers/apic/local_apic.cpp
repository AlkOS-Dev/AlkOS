#include "drivers/apic/local_apic.hpp"
#include "memory_io.hpp"

#include <assert.h>
#include <drivers/pic8259/pic8259.hpp>
#include <extensions/debug.hpp>
#include <todo.hpp>

void EnableLocalAPIC()
{
    R_ASSERT_TRUE(IsApicSupported(), "APIC is not supported on this platform...");

    /* Disable PIC unit */
    Pic8259Disable();

    TODO_WHEN_VMEM_WORKS
    /* Map local apic address to vmem */
    // TODO: currently: identity

    TRACE_INFO("Local APIC found at address: %016X", GetLocalApicPhysicalAddress());

    /* Enable apics */
}
