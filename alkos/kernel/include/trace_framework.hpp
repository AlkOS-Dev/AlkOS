#ifndef ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_
#define ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_

#include <extensions/defines.hpp>

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
    kTime,
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

// ------------------------------
// Helpers
// ------------------------------

#define CREATE_HELPER(type, module, level, name)                                           \
    template <class... Args>                                                               \
    WRAP_CALL void name(const char *format, Args... args)                                  \
    {                                                                                      \
        trace::Write<                                                                      \
            trace::TraceType::type, trace::TraceModule::module, trace::TraceLevel::level>( \
            format, args...                                                                \
        );                                                                                 \
    }

#define CREATE_DEBUG_HELPERS(module, module_name)                            \
    CREATE_HELPER(kDebugOnly, module, kFatalOnly, DEBUG_FATAL_##module_name) \
    CREATE_HELPER(kDebugOnly, module, kWarnings, DEBUG_WARN_##module_name)   \
    CREATE_HELPER(kDebugOnly, module, kInfo, DEBUG_INFO_##module_name)       \
    CREATE_HELPER(kDebugOnly, module, kFrequentInfo, DEBUG_FREQ_INFO_##module_name)

#define CREATE_TRACE_HELPERS(module, module_name)                            \
    CREATE_HELPER(kKernelLog, module, kFatalOnly, TRACE_FATAL_##module_name) \
    CREATE_HELPER(kKernelLog, module, kWarnings, TRACE_WARN_##module_name)   \
    CREATE_HELPER(kKernelLog, module, kInfo, TRACE_INFO_##module_name)       \
    CREATE_HELPER(kKernelLog, module, kFrequentInfo, TRACE_FREQ_INFO_##module_name)

// MEMORY
CREATE_DEBUG_HELPERS(kMemory, MEMORY)
CREATE_TRACE_HELPERS(kMemory, MEMORY)

// INTERRUPTS
CREATE_DEBUG_HELPERS(kInterrupts, INTERRUPTS)
CREATE_TRACE_HELPERS(kInterrupts, INTERRUPTS)

// BOOT
CREATE_DEBUG_HELPERS(kBoot, BOOT)
CREATE_TRACE_HELPERS(kBoot, BOOT)

// GENERAL
CREATE_DEBUG_HELPERS(kGeneral, GENERAL)
CREATE_TRACE_HELPERS(kGeneral, GENERAL)

// TIME
CREATE_DEBUG_HELPERS(kTime, TIME)
CREATE_TRACE_HELPERS(kTime, TIME)

#include "trace_framework.tpp"

#endif  // ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_HPP_
