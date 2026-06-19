// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <modules/global_state.hpp>
#include "trace_framework.hpp"

internal::GlobalStateModule::GlobalStateModule() noexcept
    : Settings_(global_state_constants::kDefaultGlobalSettings)
{
    DEBUG_INFO_GENERAL("Initialized the global state module");
}
