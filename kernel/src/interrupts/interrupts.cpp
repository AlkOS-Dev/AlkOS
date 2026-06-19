// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "modules/hardware.hpp"
// ------------------------------
// Assembly interface
// ------------------------------

extern "C" void *cdecl_HandleException(const u16 lirq, hal::ExceptionData *data)
{
    return HardwareModule::Get().GetInterrupts().GetLit().HandleInterrupt(lirq, data);
}

extern "C" void *cdecl_HandleHardwareInterrupt(const u16 lirq, hal::ExceptionData *data)
{
    return HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .HandleInterrupt<intr::InterruptType::kHardwareInterrupt>(lirq, data);
}

extern "C" void *cdecl_HandleSoftwareInterrupt(const u16 lirq, hal::ExceptionData *data)
{
    return HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .HandleInterrupt<intr::InterruptType::kSoftwareInterrupt>(lirq, data);
}

// ------------------------------
// Implementations
// ------------------------------

namespace intr
{

}  // namespace intr
