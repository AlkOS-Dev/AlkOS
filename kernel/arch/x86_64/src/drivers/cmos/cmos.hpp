// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_SRC_DRIVERS_CMOS_CMOS_HPP_
#define KERNEL_ARCH_X86_64_SRC_DRIVERS_CMOS_CMOS_HPP_

#include <types.h>
#include <defines.hpp>
#include "include/io.hpp"

static constexpr u64 kCmosAddressPort = 0x70;
static constexpr u64 kCmosDataPort    = 0x71;

void SetCenturyRegisterAddress(u64 address);

FAST_CALL byte ReadCmosRegister(const byte reg)
{
    outb(kCmosAddressPort, reg);
    return inb(kCmosDataPort);
}

#endif  // KERNEL_ARCH_X86_64_SRC_DRIVERS_CMOS_CMOS_HPP_
