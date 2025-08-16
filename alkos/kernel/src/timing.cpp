#include "modules/timing.hpp"
#include <extensions/debug.hpp>
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "time.hpp"

internal::TimingModule::TimingModule() noexcept
{
    ::GlobalStateModule::Get()
        .GetSettings()
        .RegisterEvent<global_state_constants::SettingsType::kIsDayTimeClockInUTC>(&OnIsUtcChanged);

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

void internal::TimingModule::OnIsUtcChanged() noexcept
{
    ::TimingModule::Get().GetDayTime().SyncWithHardware();
}
