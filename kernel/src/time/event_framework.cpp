#include "event_framework.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"

// ------------------------------
// Implementations
// ------------------------------

namespace timing
{
void EventFramework::InstallInterruptHandler() {}

void EventFramework::SetupPeriodic(const u64 time_ns) {}
}  // namespace timing
