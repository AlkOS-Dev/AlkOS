#ifndef KERNEL_SRC_HAL_PANIC_HPP_
#define KERNEL_SRC_HAL_PANIC_HPP_

#include <hal/impl/panic.hpp>
#include "modules/hardware.hpp"
#include "trace_framework.hpp"

#include <stdio.h>

namespace hal
{

/**
 * @brief Stops the kernel from functioning and disables all necessary devices and processes.
 * @note This function should also dump relevant debug information to the terminal
 *       to help diagnose the issue.
 * @param msg A message providing additional information about the panic.
 */
WRAP_CALL void KernelPanic(const char *msg)
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    HardwareModule::Get().GetCoresController().PanicAllCores();

    // For some reason trace::DumpAllBuffersOnFailure() must be called before and after printing
    trace::DumpAllBuffersOnFailure();
    TRACE_FATAL_GENERAL("[ KERNEL PANIC ]");
    TRACE_FATAL_GENERAL(msg);
    TRACE_FATAL_GENERAL("\n");

    arch::KernelPanic();
}

template <typename... Args>
FAST_CALL NO_RET void KernelPanicFormat(const char *fmt, Args... args)
{
    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    HardwareModule::Get().GetCoresController().PanicAllCores();

    // For some reason trace::DumpAllBuffersOnFailure() must be called before and after printing
    trace::DumpAllBuffersOnFailure();
    TRACE_FATAL_GENERAL("[ KERNEL PANIC ]");
    TRACE_FATAL_GENERAL(fmt, args...);
    arch::KernelPanic();

    __builtin_unreachable();
}

}  // namespace hal

#endif  // KERNEL_SRC_HAL_PANIC_HPP_
