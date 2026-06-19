// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "sys/terminal.hpp"
#include "hw/serial/qemu.hpp"

void TerminalInit()
{
    /* Initialize VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalInit();

    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalInit();
    }
}
