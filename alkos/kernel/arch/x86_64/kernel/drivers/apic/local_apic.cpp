#include "drivers/apic/local_apic.hpp"

#include <assert.h>
#include <drivers/pic8259/pic8259.hpp>
#include <extensions/debug.hpp>
#include <todo.hpp>

using namespace LocalApic;

void LocalApic::Enable()
{
    R_ASSERT_TRUE(IsSupported(), "APIC is not supported on this platform...");

    /* Disable PIC unit */
    Pic8259Disable();

    TODO_WHEN_VMEM_WORKS
    /* Map local apic address to vmem */
    // TODO: currently: identity

    TRACE_INFO("Local APIC found at address: %016X", GetPhysicalAddress());

    /* Enable Local Apic by ENABLE flag added to address (Might be enabled or might be not) */
    SetPhysicalAddress(GetPhysicalAddress());

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */

    auto reg    = CastRegister<SpuriousInterruptRegister>(ReadRegister(kSpuriousInterruptRegRW));
    reg.enabled = 1;

    WriteRegister(kSpuriousInterruptRegRW, ToRawRegister(reg));
}
