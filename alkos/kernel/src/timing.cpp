#include "modules/timing.hpp"
#include <extensions/debug.hpp>
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "time.hpp"

internal::TimingModule::TimingModule() noexcept
{
    arch::PickSystemClockSource();
    auto& clock_source = ::HardwareModule::Get().GetClockRegistry().GetActive();

    if (clock_source.enable_device) {
        clock_source.enable_device(&clock_source);
    }

    if (clock_source.resume_counter) {
        clock_source.resume_counter(&clock_source);
    }

    TRACE_INFO("TimingModule::TimingModule()");
}
