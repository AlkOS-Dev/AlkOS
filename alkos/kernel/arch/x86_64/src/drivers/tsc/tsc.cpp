#include "drivers/tsc/tsc.hpp"

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
    // TODO: add to the infra
}
