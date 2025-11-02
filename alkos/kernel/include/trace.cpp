#include <extensions/algorithm.hpp>

#include "hardware/core_local.hpp"
#include "modules/hardware.hpp"
#include "modules/timing.hpp"
#include "trace_framework.hpp"

#include <extensions/array.hpp>
#include <hal/debug_terminal.hpp>
#include <hal/terminal.hpp>

using namespace trace;

// ------------------------------
// Helper static functions
// ------------------------------

FAST_CALL void HardenFullString(const char *str)
{
    TODO_BY_THE_END_OF_MILESTONE0
    hal::TerminalWriteString(str);
}

FAST_CALL void EnsureEos(char *str, const size_t trace_size)
{
    ASSERT_NOT_ZERO(trace_size);
    const size_t eos_index = std::max(
        trace_size - 1, static_cast<size_t>(FeatureValue<FeatureFlag::kSingleTraceMaxSize>)
    );
    str[eos_index] = '\0';
}

FAST_CALL u32 GetInterruptNestingCounter()
{
    if (::HardwareModule::Get().GetCoresController().AreCoresKnown()) {
        return ::HardwareModule::Get()
            .GetCoresController()
            .GetCurrentCore()
            .GetInterruptNestingLevel();
    }
}

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

    static constexpr std::array<const char *, static_cast<size_t>(TraceModule::kLast)>
        kTraceModuleNames = []() constexpr {
            std::array<const char *, static_cast<size_t>(TraceModule::kLast)> rv{};

            rv[static_cast<size_t>(TraceModule::kBoot)]       = "BootModule";
            rv[static_cast<size_t>(TraceModule::kGeneral)]    = "GeneralModule";
            rv[static_cast<size_t>(TraceModule::kMemory)]     = "MemoryModule";
            rv[static_cast<size_t>(TraceModule::kInterrupts)] = "InterruptsModule";

            return rv;
        }();

    // ------------------------------
    // Helper types
    // ------------------------------

    struct SmallTraceCyclicBuffer {
        static constexpr size_t kSize     = 65536;
        static constexpr size_t kDumpSize = 65536 / 4;

        u32 bytes_left = kSize;
        u32 head{};
        u32 tail{};
        char buffer[kSize]{};
    };

    struct MultithreadTraceCyclicBuffer {
        u32 head;
        u32 tail;
        char *buffer;
    };

    struct CoreTraceData {
        char main_execution_workspace[FeatureValue<FeatureFlag::kSingleTraceMaxSize>];
        char interrupt_workspace[kMaxInterruptNestingAllowed]
                                [FeatureValue<FeatureFlag::kSingleTraceMaxSize>];
    };

    struct StageCallbacks {
        char *(TraceFramework::*get_workspace_cb)();
        void (TraceFramework::*commit_to_log_cb)(size_t);
        void (TraceFramework::*commit_to_debug_log_cb)(size_t);
    };

    // ------------------------------
    // Implementation details
    // ------------------------------

    FORCE_INLINE_F void AdvanceStage()
    {
        /* Assumes only one execution may call this */
        ASSERT_LT(stage, TracingStage::kLast);

        switch (stage) {
            case TracingStage::kSingleThreadEnv:
                AdvanceToSingleThreadInterruptsStage();
                stage = TracingStage::kSingleThreadInterruptsEnv;
                break;

            case TracingStage::kSingleThreadInterruptsEnv:
                AdvanceToMultiThreadStage();
                stage = TracingStage::kMultiThreadEnv;
                break;

            case TracingStage::kMultiThreadEnv:
            case TracingStage::kLast:
                R_FAIL_ALWAYS("Cannot advance tracing stage beyond multi thread env...");
        }
    }

    FORCE_INLINE_F void AdvanceToSingleThreadInterruptsStage()
    {
        stage_callbacks.get_workspace_cb = &TraceFramework::GetWorkspaceSingleThreadInterrupts;
        stage_callbacks.commit_to_log_cb = &TraceFramework::CommitToLogSingleThreadInterrupts;
        stage_callbacks.commit_to_debug_log_cb =
            &TraceFramework::CommitToDebugLogSingleThreadInterrupts;
    }

    FORCE_INLINE_F void AdvanceToMultiThreadStage() {}

    // --------------------------------------
    // Single thread env implementation
    // --------------------------------------

    char *GetWorkspaceSingleThread() { return single_thread_env.main_execution_workspace; }

    void CommitToLogSingleThread(const size_t trace_size)
    {
        EnsureEos(single_thread_env.main_execution_workspace, trace_size);

        /* Bypass kernel log as we are only writers and we do not care bout perf here */
        HardenFullString(single_thread_env.main_execution_workspace);
    }

    void CommitToDebugLogSingleThread(const size_t trace_size)
    {
        EnsureEos(single_thread_env.main_execution_workspace, trace_size);

        hal::DebugTerminalWrite(single_thread_env.main_execution_workspace);
    }

    // -------------------------------------------------
    // Single thread interrupts env implementation
    // -------------------------------------------------

    void CommitToLogPtrSingleThreadInterrupts(
        SmallTraceCyclicBuffer &buffer, const size_t trace_size
    )
    {
        const u8 nested_intrs = hardware::GetCoreLocalData().nested_interrupts;

        if (nested_intrs == 0 && buffer.bytes_left < SmallTraceCyclicBuffer::kDumpSize) {
            // Dump the content
        }
    }

    char *GetWorkspaceSingleThreadInterrupts()
    {
        const u8 nested_intrs = hardware::GetCoreLocalData().nested_interrupts;

        if (nested_intrs == 0) {
            return single_thread_env.main_execution_workspace;
        }

        return single_thread_env.interrupt_workspace[nested_intrs - 1];
    }

    void CommitToLogSingleThreadInterrupts(const size_t trace_size)
    {
        CommitToLogPtrSingleThreadInterrupts(trace_log, trace_size);
    }

    void CommitToDebugLogSingleThreadInterrupts(const size_t trace_size)
    {
        CommitToLogPtrSingleThreadInterrupts(trace_debug_log, trace_size);
    }

    // ------------------------------------
    // Multithread env implementation
    // ------------------------------------

    // ------------------------------
    // Global fields
    // ------------------------------

    TracingStage stage = TracingStage::kSingleThreadEnv;

    StageCallbacks stage_callbacks{
        .get_workspace_cb       = &TraceFramework::GetWorkspaceSingleThread,
        .commit_to_log_cb       = &TraceFramework::CommitToLogSingleThread,
        .commit_to_debug_log_cb = &TraceFramework::CommitToDebugLogSingleThread,
    };

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

    CoreTraceData single_thread_env{};
    SmallTraceCyclicBuffer trace_log{};
    SmallTraceCyclicBuffer trace_debug_log{};

    // ------------------------------
    // Multi thread env fields
    // ------------------------------

    MultithreadTraceCyclicBuffer dyn_trace_log{};
    MultithreadTraceCyclicBuffer dyn_debug_trace_log{};
    CoreTraceData *multithread_env = nullptr;

} g_TraceFramework{};

namespace internal
{
char *GetWorkspace()
{
    return (g_TraceFramework.*g_TraceFramework.stage_callbacks.get_workspace_cb)();
}

void CommitToLog(const size_t trace_size)
{
    return (g_TraceFramework.*g_TraceFramework.stage_callbacks.commit_to_log_cb)(trace_size);
}

void CommitToDebugLog(const size_t trace_size)
{
    return (g_TraceFramework.*g_TraceFramework.stage_callbacks.commit_to_debug_log_cb)(trace_size);
}

size_t WriteTraceData(char *dst, TraceModule module)
{
    const char *module_name = TraceFramework::kTraceModuleNames[static_cast<size_t>(module)];

    u64 system_time = 0;
    if (::TimingModule::IsInited()) {
        system_time = ::TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    }

    u32 core_id = hal::GetCurrentCoreId();
    if (::HardwareModule::IsInited() &&
        ::HardwareModule::Get().GetCoresController().AreCoresKnown()) {
        core_id = ::HardwareModule::Get().GetCoresController().MapHwToLogical(core_id);
    }

    return snprintf(
        dst, FeatureValue<FeatureFlag::kSingleTraceMaxSize>,
        "%s "    // Module name
        "%llu "  // Timestamp
        "%u "    // Core ID
        "%u ",   // Process ID
        module_name, system_time, core_id,
        0u  // TODO: process ID
    );
}

}  // namespace internal

namespace trace
{
void AdvanceTracingStage() { g_TraceFramework.AdvanceStage(); }

NODISCARD TraceLevel GetTraceLevel(TraceModule module)
{
    return g_TraceFramework.trace_levels[static_cast<size_t>(module)];
}
}  // namespace trace
