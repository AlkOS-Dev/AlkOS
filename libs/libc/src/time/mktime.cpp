// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <alkos/calls.h>
#include <time.h>
#include <time.hpp>

// ------------------------------
// Implementation
// ------------------------------

time_t mktime(tm *time_ptr) { return MkTimeFromTimeZone(*time_ptr, GetTimezoneSysCall()); }
