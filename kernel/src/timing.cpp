#include "modules/timing.hpp"
#include "hal/timers.hpp"
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "trace_framework.hpp"

internal::TimingModule::TimingModule() noexcept
{
    ::HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();

    arch::PickSystemClockSource();
    auto &clock_source = ::HardwareModule::Get().GetClockRegistry().GetSelected();

    if (clock_source.enable_device) {
        clock_source.enable_device(&clock_source);
    }

    if (clock_source.resume_counter) {
        clock_source.resume_counter(&clock_source);
    }

    GetSystemTime().SyncWithHardware();
    ::HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();

    DEBUG_INFO_TIME("TimingModule::TimingModule()");
}
