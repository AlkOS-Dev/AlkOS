// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_DEBUG_TERMINAL_HPP_
#define KERNEL_SRC_HAL_DEBUG_TERMINAL_HPP_

#include <hal/impl/debug_terminal.hpp>

namespace hal
{

WRAP_CALL void DebugTerminalWrite(const char *str)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        arch::DebugTerminalWrite(str);
    }
}

WRAP_CALL void DebugTerminalPutChar(const char c)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        arch::DebugTerminalPutChar(c);
    }
}

WRAP_CALL size_t DebugTerminalReadLine(char *const buffer, const size_t buffer_size)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        return arch::DebugTerminalReadLine(buffer, buffer_size);
    }

    return 0;
}

}  // namespace hal

#endif  // KERNEL_SRC_HAL_DEBUG_TERMINAL_HPP_
