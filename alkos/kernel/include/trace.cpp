#include "trace_framework.hpp"

#include <extensions/array.hpp>

using namespace trace;

// ------------------------------
// Helper static functions
// ------------------------------

// ------------------------------------
// Trace framework implementation
// ------------------------------------

static struct TraceFramework {
    // ------------------------------
    // Constants
    // ------------------------------

    static constexpr auto kDefaultTraceLevel = TraceLevel::kInfo;
    static constexpr auto kMaxInterruptNestingAllowed =
        4; /* E.g irq -> NMI -> fault -> double fault */

    // ------------------------------
    // Helper types
    // ------------------------------

    struct TraceCyclicBuffer {
        size_t head;
        size_t tail;
        char buffer[FeatureValue<FeatureFlag::kTraceBufferSize>];
    };

    struct ThreadTraceData {
        char main_execution_workspace[FeatureValue<FeatureFlag::kSingleTraceMaxSize>];
        char interrupt_workspace[kMaxInterruptNestingAllowed]
                                [FeatureValue<FeatureFlag::kSingleTraceMaxSize>];
    };

    struct StageCallbacks {
        char *(TraceFramework::*get_workspace_cb)();
        void (TraceFramework::*commit_to_log_cb)(size_t, TraceModule);
        void (TraceFramework::*commit_to_debug_log_cb)(size_t, TraceModule);
    };

    // ------------------------------
    // Implementation details
    // ------------------------------

    // --------------------------------------
    // Single thread env implementation
    // --------------------------------------

    // -------------------------------------------------
    // Single thread interrupts env implementation
    // -------------------------------------------------

    // ------------------------------------
    // Multithread env implementation
    // ------------------------------------

    // ------------------------------
    // Global fields
    // ------------------------------

    TracingStage stage = TracingStage::kSingleThreadEnv;
    StageCallbacks stage_callbacks{};
    std::array<TraceLevel, static_cast<size_t>(TraceModule::kLast)> trace_levels = []() constexpr {
        std::array<TraceLevel, static_cast<size_t>(TraceModule::kLast)> rv{};

        for (size_t idx = 0; idx < static_cast<size_t>(TraceModule::kLast); ++idx) {
            rv[idx] = kDefaultTraceLevel;
        }

        /* Boot module */
        rv[static_cast<size_t>(TraceModule::kBoot)] =
            static_cast<TraceLevel>(FeatureValue<FeatureFlag::kBootTraceLevel>);

        /* General module */
        rv[static_cast<size_t>(TraceModule::kGeneral)] =
            static_cast<TraceLevel>(FeatureValue<FeatureFlag::kGeneralTraceLevel>);

        /* Memory module */
        rv[static_cast<size_t>(TraceModule::kMemory)] =
            static_cast<TraceLevel>(FeatureValue<FeatureFlag::kMemoryTraceLevel>);

        /* Interrupts module */
        rv[static_cast<size_t>(TraceModule::kInterrupts)] =
            static_cast<TraceLevel>(FeatureValue<FeatureFlag::kInterruptsTraceLevel>);

        return rv;
    }();

    // ------------------------------
    // Single thread env fields
    // ------------------------------

    ThreadTraceData single_thread_env;

    // ------------------------------
    // Multi thread env fields
    // ------------------------------

    ThreadTraceData *multithread_env = nullptr;

} g_TraceFramework{};

namespace internal
{
char *GetWorkspace() {}

void CommitToLog(size_t trace_size, trace::TraceModule module) {}

void CommitToDebugLog(size_t trace_size, trace::TraceModule module) {}
}  // namespace internal

namespace trace
{
NODISCARD TracingStage GetCurrentStage() { return g_TraceFramework.stage; }

void AdvanceTracingStage() {}

void SetTraceLevel(TraceModule module, TraceLevel level) { R_FAIL_ALWAYS("Not Implemented..."); }

NODISCARD TraceLevel GetTraceLevel(TraceModule module)
{
    return g_TraceFramework.trace_levels[static_cast<size_t>(module)];
}
}  // namespace trace
