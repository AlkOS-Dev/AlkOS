// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "thread.hpp"

#include "modules/timing.hpp"
#include "trace_framework.hpp"

namespace Sched
{
u64 Thread::CalculateCpuTime()
{
    const u64 time = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    kernel_time_ns += (time - timestamp);

    return (kernel_time_ns + user_time_ns) - (time - timestamp_execution_start_ns);
}
}  // namespace Sched
