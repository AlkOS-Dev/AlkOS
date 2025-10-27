#ifndef ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_
#define ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_

#include <extensions/defines.hpp>

namespace trace
{
enum class TracingStage {
    kSingleThreadEnv = 0, /* Starts on this stage */
    kMultiThreadEnv,      /* Should end up here */
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
    kLast,
};

NODISCARD TracingStage GetCurrentStage();
void AdvanceTracingStage();
void SetTraceLevel(TraceModule module, TraceLevel level);
NODISCARD TraceLevel GetTraceLevel(TraceModule module);

// template<TraceType type, TraceModule module>
// void Write()
}  // namespace trace

#endif  // ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_
