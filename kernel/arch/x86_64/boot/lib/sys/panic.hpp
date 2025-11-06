#ifndef ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_SYS_PANIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_SYS_PANIC_HPP_

#include <stdio.h>

/**
 * @brief Stops the kernel from functioning and disables all necessary devices and processes.
 * @note This function should also dump relevant debug information to the terminal
 *       to help diagnose the issue.
 * @param msg A message providing additional information about the panic.
 */
extern "C" NO_RET void KernelPanic(const char *msg);

template <typename... Args>
FAST_CALL NO_RET void KernelPanicFormat(const char *fmt, Args... args)
{
    static constexpr size_t kKernelPanicPrintBuffSize = 2048;
    char buffer[kKernelPanicPrintBuffSize];

    snprintf(buffer, kKernelPanicPrintBuffSize, fmt, args...);

    KernelPanic(buffer);
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_SYS_PANIC_HPP_
