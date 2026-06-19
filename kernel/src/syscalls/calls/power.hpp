// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SYSCALLS_CALLS_POWER_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_POWER_HPP_

#include "acpi/acpi_power.hpp"
#include "alkos/power.h"

#include <defines.h>

namespace Syscall
{

FORCE_INLINE_F void SysPower(PowerAction action)
{
    switch (action) {
        case kShutdown:
            ACPI::SystemShutdown();
            break;
        case kReboot:
            ACPI::SystemReboot();
            break;
    }
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_POWER_HPP_
