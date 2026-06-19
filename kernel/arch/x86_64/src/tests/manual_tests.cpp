// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <test_module/test.hpp>
#include "cpu/utils.hpp"

#include "modules/hardware.hpp"

/**
 * Simply prints the default exception message.
 */
MTEST(VerifyDefaultExceptionMsg)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
    [[maybe_unused]] volatile int a = 9 / 0;
#pragma GCC diagnostic pop
}

static constexpr u64 kTimerInterval = 5'000'000;  // 5ms
static Sched::Thread *TimerHandler(intr::LitHwEntry &)
{
    TRACE_FATAL_ACPI("ACK FROM HANDLER!");
    trace::DumpAllBuffersOnFailure();

    auto &event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    const u32 result  = event_clock.cbs.next_event(&event_clock, kTimerInterval);
    ASSERT_ZERO(result);

    return nullptr;
}

MTEST(LAPIC)
{
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
            hal::kTimerHwLirq, intr::HwHandler{.handler = TimerHandler}
        );

    auto &event_clock = HardwareModule::Get().GetEventClockRegistry().GetSelected();
    ASSERT_TRUE(event_clock.flags.IsCoreLocal, "Scheduler supports only core local event clocks");
    event_clock.cbs.set_oneshot(&event_clock);

    event_clock.cbs.next_event(&event_clock, kTimerInterval);

    for (size_t i = 0; i < 10; ++i) {
        HaltCpu();
    }
}
