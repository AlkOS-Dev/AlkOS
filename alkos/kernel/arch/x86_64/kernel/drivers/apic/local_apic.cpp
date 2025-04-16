#include "drivers/apic/local_apic.hpp"
#include "msrs.hpp"

#include <assert.h>

void EnableLocalAPIC()
{
    R_ASSERT_TRUE(IsApicSupported(), "APIC is not supported on this platform...");
}
