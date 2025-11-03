#ifndef ALKOS_KERNEL_INCLUDE_HAL_PANIC_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_PANIC_HPP_

#include <hal/impl/panic.hpp>

#include <stdio.h>

namespace hal
{

/**
 * @brief Stops the kernel from functioning and disables all necessary devices and processes.
 * @note This function should also dump relevant debug information to the terminal
 *       to help diagnose the issue.
 * @param msg A message providing additional information about the panic.
 */
WRAP_CALL void KernelPanic(const char *msg) { arch::KernelPanic(msg); }

template <typename... Args>
FAST_CALL NO_RET void KernelPanicFormat(const char *fmt, Args... args)
{
    static constexpr size_t kKernelPanicPrintBuffSize = 2048;
    char buffer[kKernelPanicPrintBuffSize];

    snprintf(buffer, kKernelPanicPrintBuffSize, fmt, args...);

    KernelPanic(buffer);
    __builtin_unreachable();
}

}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_PANIC_HPP_
