#include "event_framework.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"

// ------------------------------
// statics
// ------------------------------

static Sched::Thread *TimerHandler(intr::LitHwEntry &)
{
    return SchedulingModule::Get().GetScheduler().Schedule();
}

// ------------------------------
// Implementations
// ------------------------------

namespace timing
{
void EventFramework::InstallInterruptHandler()
{
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
            hal::kTimerHwLirq, intr::HwHandler{.handler = TimerHandler}
        );
}

void EventFramework::SetupPeriodic(const u64 time_ns)
{
    ASSERT_TRUE(HardwareModule::Get().GetEventClockRegistry().IsSelectedPicked());

    auto event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    event_clock.cbs.set_periodic(&event_clock);
    event_clock.cbs.next_event(&event_clock, time_ns);
}
}  // namespace timing
