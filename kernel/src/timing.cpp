// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "modules/timing.hpp"
#include "hal/timers.hpp"
#include "modules/global_state.hpp"
#include "modules/hardware.hpp"
#include "trace_framework.hpp"

internal::TimingModule::TimingModule() noexcept
{
    ::HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();

    // 1. Prepare System time clock source
    hal::PickSystemClockSource();
    auto &clock_source = ::HardwareModule::Get().GetClockRegistry().GetSelected();

    if (clock_source.enable_device) {
        clock_source.enable_device(&clock_source);
    }

    if (clock_source.resume_counter) {
        clock_source.resume_counter(&clock_source);
    }

    GetSystemTime().SyncWithHardware();

    // 2. Prepare system event clock source
    hal::PickSystemEventClockSource();

    ::HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();

    DEBUG_INFO_TIME("TimingModule::TimingModule()");
}
