// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <assert.h>
#include <time.h>
#include <time.hpp>

struct tm *gmtime_r(const time_t *timer, struct tm *result)
{
    return ConvertFromPosixToTm(*timer, *result, kUtcTimezone);
}

struct tm *gmtime(const time_t *timer)
{
    static tm buffer;
    return gmtime_r(timer, &buffer);
}
