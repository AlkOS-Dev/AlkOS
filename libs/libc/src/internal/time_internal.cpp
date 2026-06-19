// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <todo.h>
#include <types.h>
#include <time.hpp>
#include <time_internal.hpp>

u64 __GetLocalTimezoneOffsetNs()
{
    TODO_TIMEZONES
    /* Hard coded UTC */
    static constexpr u64 kUctOffset = 1;

    return kNanosInSecond * kSecondsInHour * kUctOffset;
}

u64 __GetDstTimezoneOffsetNs()
{
    TODO_TIMEZONES
    /* Hard coded Poland */
    static constexpr u64 kPolandOffset = 1;

    return kNanosInSecond * kSecondsInHour * kPolandOffset;
}
