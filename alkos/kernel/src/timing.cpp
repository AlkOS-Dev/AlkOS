#include <extensions/debug.hpp>
#include <modules/global_state.hpp>
#include <modules/timing.hpp>

internal::TimingModule::TimingModule() noexcept
{
    ::GlobalStateModule::Get()
        .GetSettings()
        .RegisterEvent<global_state_constants::SettingsType::kIsDayTimeClockInUTC>(&OnIsUtcChanged);

    TRACE_INFO("TimingModule::TimingModule()");
}

void internal::TimingModule::OnIsUtcChanged() noexcept
{
    ::TimingModule::Get().day_time_.SyncWithHardware();
}
