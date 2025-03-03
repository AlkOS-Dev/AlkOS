#ifndef LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_
#define LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_

#include <todo.h>

TODO_STREAMS
/* Note: Currently works only in kernel code due to lack of libc terminal abstractions */
#if defined(__USE_DEBUG_TRACES__) && defined(__ALKOS_KERNEL__)

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

static constexpr size_t kTraceBufferSize = 4096;
template <typename... Args>
FAST_CALL constexpr void FormatTrace(const char* format, Args... args)
{
    char buffer[kTraceBufferSize];

    const u64 bytesWritten = snprintf(buffer, kTraceBufferSize, format, args...);
    ASSERT(bytesWritten < kTraceBufferSize);
    DebugTerminalWrite(buffer);
}

#define TRACE(message, ...) FormatTrace(message __VA_OPT__(, ) __VA_ARGS__)

#else

#define TRACE(message, ...)

#endif  // __USE_DEBUG_TRACES__

#define TRACE_FORMAT_LOCATION(message) __FILE__ " " TOSTRING(__LINE__) " " message "\n"
#define TRACE_FORMAT_ERROR(message)    "[ERROR]     " TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_WARNING(message)  "[WARNING]   " TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_INFO(message)     "[INFO]      " TRACE_FORMAT_LOCATION(message)
#define TRACE_FORMAT_SUCCESS(message)  "[SUCCESS]   " TRACE_FORMAT_LOCATION(message)

#define TRACE_ERROR(message, ...)   TRACE(TRACE_FORMAT_ERROR(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_WARNING(message, ...) TRACE(TRACE_FORMAT_WARNING(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_INFO(message, ...)    TRACE(TRACE_FORMAT_INFO(message) __VA_OPT__(, ) __VA_ARGS__)
#define TRACE_SUCCESS(message, ...) TRACE(TRACE_FORMAT_SUCCESS(message) __VA_OPT__(, ) __VA_ARGS__)

#endif  // LIBC_INCLUDE_EXTENSIONS_DEBUG_HPP_
