#ifndef ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_
#define ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_

#include <extensions/defines.hpp>

#define FORMAT(message) __FILE__ " " TOSTRING(__LINE__) " " message

namespace trace
{
// ------------------------------
// Type definitions
// ------------------------------

enum class TracingStage {
    kSingleThreadEnv = 0, /* Starts on this stage */
    kSingleThreadInterruptsEnv,
    kMultiThreadEnv, /* Should end up here */
    kLast,
};

enum class TraceLevel {
    kFatalOnly = 0,
    kWarnings,
    kInfo,
    kFrequentInfo,
    kLast,
};

enum class TraceType {
    kDebugOnly = 0,
    kKernelLog,
    kShell,
    kLast,
};

enum class TraceModule {
    kMemory = 0,
    kInterrupts,
    kBoot,
    kGeneral,
    kLast,
};

// ------------------------------
// User API
// ------------------------------

void AdvanceTracingStage();
NODISCARD TraceLevel GetTraceLevel(TraceModule module);
void DumpAllBuffersOnFailure();

/* MODULE TIME CORE PROC FILE LINE MSG */
template <TraceType type, TraceModule module, TraceLevel level, class... Args>
FAST_CALL void Write(const char *format, Args... args);
}  // namespace trace

#define KERNEL_TRACE(type, module, level, message, ...) \
    trace::Write<type, module, level>(FORMAT(message) __VA_OPT__(, ) __VA_ARGS__)

#include "trace_framework.tpp"

#endif  // ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_
