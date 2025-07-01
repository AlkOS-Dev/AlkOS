#ifndef ALKOS_KERNEL_INCLUDE_TRACE_HPP_
#define ALKOS_KERNEL_INCLUDE_TRACE_HPP_

#include <stdio.h>
#include <extensions/debug.hpp>

template <class... Args>
void KernelTrace(const char* format, Args... args)
{
    static constexpr size_t kTraceBufferSize = 4096;
    char buffer[kTraceBufferSize];

    [[maybe_unused]] const u64 bytesWritten = snprintf(buffer, kTraceBufferSize, format, args...);
    ASSERT(bytesWritten < kTraceBufferSize);
    DebugTerminalWrite(buffer);
}

#define KernelTraceSuccess(format, ...) \
    KernelTrace(SUCCESS_TAG format "\n" __VA_OPT__(, ) __VA_ARGS__)
#define KernelTraceError(format, ...) KernelTrace(ERROR_TAG format "\n" __VA_OPT__(, ) __VA_ARGS__)
#define KernelTraceInfo(format, ...)  KernelTrace(INFO_TAG format "\n" __VA_OPT__(, ) __VA_ARGS__)
#define KernelTraceWarning(format, ...) \
    KernelTrace(WARNING_TAG format "\n" __VA_OPT__(, ) __VA_ARGS__)

#endif  // ALKOS_KERNEL_INCLUDE_TRACE_HPP_
