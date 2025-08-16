#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_

#include <autogen/feature_flags.h>
#include <todo.h>

// Allow debug traces only when we run inside kernel
#if FEATURE_FLAG_DEBUG_TRACES && (defined(__ALKOS_KERNEL__) || defined(__ALKOS_LIBK__))

#include <assert.h>
#include <stdio.h>
#include <debug_terminal.hpp>
#include <extensions/defines.hpp>
#include <extensions/time.hpp>
#include <todo.hpp>

// ------------------------------
// Traces
// ------------------------------

static constexpr size_t kTraceBufferSize = 4096;
template <typename... Args>
void FormatTrace(const char* format, Args... args)
{
    TODO_WHEN_TIMER_INFRA_DONE
    // TODO: common loader does not have access to kernel structures
    // TODO: -> common loader should use different traces
    // TODO: without that kernel traces will not display time of the trace

    char buffer[kTraceBufferSize];
    [[maybe_unused]] const u64 bytesWritten =
        snprintf(buffer, kTraceBufferSize, format, 0, args...);
    ASSERT(bytesWritten < kTraceBufferSize);
    DebugTerminalWrite(buffer);
}

#define TRACE(message, ...) FormatTrace(message __VA_OPT__(, ) __VA_ARGS__)

#else

#define TRACE(message, ...)

#endif  // __USE_DEBUG_TRACES__

#define ERROR_TAG   "[ERROR]     "
#define WARNING_TAG "[WARNING]   "
#define INFO_TAG    "[INFO]      "
#define SUCCESS_TAG "[SUCCESS]   "
#define DEBUG_TAG   "[DEBUG]     "
#define TRACE_TAG   "[TRACE]     "

#define TRACE_FORMAT_LOCATION(message) __FILE__ " " TOSTRING(__LINE__) " " message "\n"
#define TRACE_FORMAT_ERROR(message)    ERROR_TAG " %zu " TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_WARNING(message)  WARNING_TAG " %zu " TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_INFO(message)     INFO_TAG " %zu " TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_DEBUG(message)    DEBUG_TAG " %zu " TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_SUCCESS(message)  SUCCESS_TAG " %zu " TRACE_FORMAT_LOCATION(message)

#define TRACE_ERROR(message, ...)   TRACE(TRACE_FORMAT_ERROR(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_WARNING(message, ...) TRACE(TRACE_FORMAT_WARNING(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_INFO(message, ...)    TRACE(TRACE_FORMAT_INFO(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_DEBUG(message, ...)   TRACE(TRACE_FORMAT_DEBUG(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_SUCCESS(message, ...) TRACE(TRACE_FORMAT_SUCCESS(message) __VA_OPT__(, ) __VA_ARGS__)

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_
