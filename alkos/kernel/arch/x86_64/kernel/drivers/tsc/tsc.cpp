#include "drivers/tsc/tsc.hpp"

void tsc::Initialize()
{
    if (!IsAvailable()) {
        KernelTraceInfo("TSC is not available. Fallback to old technology...");
        return;
    }

    // NOTE: disabled RDTSC in user space
    SetUserSpaceAccess(false);

    TRACE_DEBUG("Detected TSC, current counter: %zu", Read());

    TODO_WHEN_TIMER_INFRA_DONE
    // TODO: add to the infra
}
