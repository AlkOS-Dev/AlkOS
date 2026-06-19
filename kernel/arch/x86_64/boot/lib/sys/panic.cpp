// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "sys/panic.hpp"
#include "cpu/utils.hpp"
#include "sys/terminal.hpp"

extern "C" void NO_RET KernelPanic(const char *msg)
{
    TerminalWriteError("[ KERNEL PANIC ]\n");
    TerminalWriteError(msg);
    TerminalWriteError("\n");
    OsHang();
}
