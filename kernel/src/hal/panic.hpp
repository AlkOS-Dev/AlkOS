// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_PANIC_HPP_
#define KERNEL_SRC_HAL_PANIC_HPP_

#include <hal/impl/panic.hpp>
#include "modules/hardware.hpp"
#include "trace_framework.hpp"

#include <stdio.h>
#include "scheduling/local_lock.hpp"

namespace hal
{

/**
 * @brief Stops the kernel from functioning and disables all necessary devices and processes.
 * @note This function should also dump relevant debug information to the terminal
 *       to help diagnose the issue.
 * @param msg A message providing additional information about the panic.
 */
FAST_CALL void KernelPanic(const char *msg)
{
    if (HardwareModule::IsInited()) {
        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
        HardwareModule::Get().GetCoresController().PanicAllCores();
    }
    trace::DumpAllBuffersOnFailure();

    // BYPASS TRACE FRAMEWORK AS this function is used inside of it
    TerminalWriteString("[ KERNEL PANIC ]");
    TerminalWriteString(msg);
    TerminalWriteString("\n");

    arch::KernelPanic();
    __builtin_unreachable();
}

template <typename... Args>
FAST_CALL NO_RET void KernelPanicFormat(const char *fmt, Args... args)
{
    static constexpr size_t kMaxSize = 2048;
    char buff[kMaxSize];

    if (HardwareModule::IsInited()) {
        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
        HardwareModule::Get().GetCoresController().PanicAllCores();
    }

    // For some reason trace::DumpAllBuffersOnFailure() must be called before and after printing
    trace::DumpAllBuffersOnFailure();

    TerminalWriteString("[ KERNEL PANIC ] ");
    snprintf(buff, kMaxSize, fmt, args...);
    TerminalWriteString(buff);
    TerminalWriteString("\n");

    arch::KernelPanic();
    __builtin_unreachable();
}

}  // namespace hal

#endif  // KERNEL_SRC_HAL_PANIC_HPP_
