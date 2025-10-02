#ifndef ALKOS_KERNEL_INCLUDE_HAL_PANIC_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_PANIC_HPP_

#include <hal/impl/panic.hpp>

#include <stdio.h>

namespace hal
{

WRAP_CALL NO_RET void KernelPanic(const char *msg) { arch::KernelPanic(msg); }

template <typename... Args>
FAST_CALL NO_RET void KernelPanicFormat(const char *fmt, Args... args)
{
    static constexpr size_t kKernelPanicPrintBuffSize = 2048;
    char buffer[kKernelPanicPrintBuffSize];

    snprintf(buffer, kKernelPanicPrintBuffSize, fmt, args...);

    arch::KernelPanic(buffer);
}

}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_PANIC_HPP_
