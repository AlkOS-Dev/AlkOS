#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_

#include <todo.h>

#if defined(__USE_DEBUG_TRACES__)

#include <assert.h>
#include <stdio.h>
#include <debug_terminal.hpp>
#include <defines.hpp>

// ------------------------------
// Traces
// ------------------------------

/**
 * @brief TRACE - Macro to print some debug information only when build is instructed to.
 * Works only when __USE_DEBUG_OUTPUT__ and __USE_DEBUG_TRACES__ are defined.
 */

#error XDXD
static constexpr size_t kTraceBufferSize = 4096;
template <typename... Args>
void FormatTrace(const char* format, Args... args)
{
    char buffer[kTraceBufferSize];

    [[maybe_unused]] const u64 bytesWritten = snprintf(buffer, kTraceBufferSize, format, args...);
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
#define TRACE_FORMAT_ERROR(message)    ERROR_TAG TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_WARNING(message)  WARNING_TAG TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_INFO(message)     INFO_TAG TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_DEBUG(message)    DEBUG_TAG TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_SUCCESS(message)  SUCCESS_TAG TRACE_FORMAT_LOCATION(message)

#define TRACE_ERROR(message, ...)   TRACE(TRACE_FORMAT_ERROR(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_WARNING(message, ...) TRACE(TRACE_FORMAT_WARNING(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_INFO(message, ...)    TRACE(TRACE_FORMAT_INFO(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_DEBUG(message, ...)   TRACE(TRACE_FORMAT_DEBUG(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_SUCCESS(message, ...) TRACE(TRACE_FORMAT_SUCCESS(message) __VA_OPT__(, ) __VA_ARGS__)

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_
