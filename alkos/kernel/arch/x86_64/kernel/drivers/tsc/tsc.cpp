#include "drivers/tsc/tsc.hpp"
#include "modules/hardware.hpp"

// ------------------------------
// Static functions
// ------------------------------

// ------------------------------
// Implementations
// ------------------------------

void tsc::Initialize()
{
    if (!IsAvailable()) {
        KernelTraceWarning("TSC is not available. Fallback to old technology...");
        return;
    }

    // NOTE: disabled RDTSC in user space
    SetUserSpaceAccess(false);

    TRACE_DEBUG("Detected TSC, current counter: %zu", Read());

    if (!IsStable()) {
        KernelTraceWarning("TSC is not stable. Fallback to old technology...");
    }

    TODO_WHEN_TIMER_INFRA_DONE
    hardware::ClockRegistryEntry tsc_entry = {};
    tsc_entry.id                           = static_cast<u64>(arch::HardwareClockId::kTsc);
    // tsc_entry.frequency_kHz
    // tsc_entry.uncertainty_margin_per_sec
}
