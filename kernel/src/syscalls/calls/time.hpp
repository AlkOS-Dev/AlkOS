// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SYSCALLS_CALLS_TIME_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_TIME_HPP_

#include <alkos/time.h>
#include <defines.h>
#include <types.h>

#include "modules/timing.hpp"
#include "modules/timing_constants.hpp"

namespace Syscall
{
// ------------------------------
// Time Syscalls
// ------------------------------

/**
 * @brief Get clock value
 * @param type Clock type to query
 * @param time Output time value
 * @param time_zone Output timezone
 */
FORCE_INLINE_F void SysGetClockValue(ClockType type, TimeVal *time, Timezone *time_zone)
{
    ASSERT_NOT_NULL(time);
    ASSERT_NOT_NULL(time_zone);

    if (!TimingModule::IsInited()) {
        if (time != nullptr) {
            time->seconds   = 0;
            time->remainder = 0;
        }
        return;
    }

    if (time_zone != nullptr) {
        __platform_get_timezone(time_zone);
    }

    if (time != nullptr) {
        switch (type) {
            case kTimeUtc: {
                time->seconds   = TimingModule::Get().GetSystemTime().Read();
                time->remainder = 0;
            } break;
            case kProcTime: {  // In microseconds
                time->seconds   = 0;
                time->remainder = timing::SystemTime::ReadLifeTimeNs() / 1000;
            } break;
            case kProcTimePrecise: {  // In nanoseconds
                time->seconds   = 0;
                time->remainder = timing::SystemTime::ReadLifeTimeNs();
            } break;
            default:
                R_FAIL_ALWAYS("Provided invalid ClockType!");
        }
    }
}

/**
 * @brief Get timezone information
 * @param time_zone Output timezone
 */
FORCE_INLINE_F void SysGetTimezone(Timezone *time_zone)
{
    ASSERT_NOT_NULL(time_zone);

    ASSERT_TRUE(TimingModule::IsInited(), "Timing module is not initialized");
    *time_zone = TimingModule::Get().GetSystemTime().GetTimezone();
}

/**
 * @brief Get clock ticks per second for a given clock type
 * @param type Clock type
 * @return Ticks per second
 */
FORCE_INLINE_F u64 SysGetClockTicksInSecond(ClockType type)
{
    const auto idx = static_cast<size_t>(type);
    ASSERT(idx != 0 && idx < timing_constants::kClockTicksInSecondSize);

    TODO_USERSPACE
    //    if (idx == 0 || idx >= kResoSize) {
    //        return 0;
    //    }

    return timing_constants::kClockTicksInSecond[idx];
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_TIME_HPP_
