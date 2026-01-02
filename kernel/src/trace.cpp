#include <algorithm.hpp>

#include "hardware/core_local.hpp"
#include "modules/hardware.hpp"
#include "modules/timing.hpp"
#include "trace_framework.hpp"

#include <array.hpp>
#include <hal/debug_terminal.hpp>
#include <hal/terminal.hpp>

using namespace trace;

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
            rv[static_cast<size_t>(TraceModule::kTime)]       = "TimeModule";
            rv[static_cast<size_t>(TraceModule::kVfs)]        = "VFSModule";
            rv[static_cast<size_t>(TraceModule::kVideo)]      = "VideoModule";
            rv[static_cast<size_t>(TraceModule::kHardware)]   = "HardwareModule";
            rv[static_cast<size_t>(TraceModule::kAcpi)]       = "ACPIModule";
            rv[static_cast<size_t>(TraceModule::kScheduling)] = "SchedulingModule";

            return rv;
        }();

    // ------------------------------
    // Helper types
    // ------------------------------

    struct SmallTraceCyclicBuffer {
        static constexpr size_t kSize      = FeatureValue<FeatureFlag::kTraceBufferSize>;
        static constexpr size_t kBatchSize = 512;

        hal::Atomic32 bytes_left{.value = kSize};
        hal::Atomic32 head{};
        hal::Atomic32 tail{};
        char buffer[kSize]{};
    };

    struct MultiCoreTraceCyclicBuffer {
        static constexpr size_t kSize = FeatureValue<FeatureFlag::kTraceBufferSize>;

        hal::Atomic32 bytes_left{.value = kSize};
        hal::Atomic32 head{};
        hal::Atomic32 tail{};
        char buffer[kSize]{};
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
        void (TraceFramework::*dump_all)();
    };

    // ------------------------------
    // Implementation details
    // ------------------------------

    FORCE_INLINE_F void AdvanceStage()
    {
        /* Assumes only one execution may call this */
        ASSERT_LT(stage, TracingStage::kLast);

        switch (stage) {
            case TracingStage::kSingleCoreEnv:
                AdvanceToSingleCoreInterruptsStage();
                stage = TracingStage::kSingleCoreInterruptsEnv;
                break;

            case TracingStage::kSingleCoreInterruptsEnv:
                AdvanceToMultiCoreStage();
                stage = TracingStage::kMultiCoreEnv;
                break;

            case TracingStage::kMultiCoreEnv:
            case TracingStage::kLast:
                R_FAIL_ALWAYS("Cannot advance tracing stage beyond multi core env...");
        }
    }

    FORCE_INLINE_F void AdvanceToSingleCoreInterruptsStage()
    {
        /* Should be executed before interrupts are enabled */

        DEBUG_INFO_GENERAL("Updating tracing stage to stage 1 (Single Core with interrupts)");
        stage_callbacks.get_workspace_cb = &TraceFramework::GetWorkspaceSingleCoreInterrupts;
        stage_callbacks.commit_to_log_cb = &TraceFramework::CommitToLogSingleCoreInterrupts;
        stage_callbacks.commit_to_debug_log_cb =
            &TraceFramework::CommitToDebugLogSingleCoreInterrupts;
        stage_callbacks.dump_all = &TraceFramework::DumpAllSingleCoreInterrupts;
    }

    FORCE_INLINE_F void AdvanceToMultiCoreStage()
    {
        /* Should be executed before another cores are enabled!!! */

        DEBUG_INFO_GENERAL("Updating tracing stage to stage 2 (Multicore environment)");
        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();

        stage_callbacks.get_workspace_cb       = &TraceFramework::GetWorkspaceMultiCore;
        stage_callbacks.commit_to_log_cb       = &TraceFramework::CommitToLogMultiCore;
        stage_callbacks.commit_to_debug_log_cb = &TraceFramework::CommitToDebugLogMultiCore;
        stage_callbacks.dump_all               = &TraceFramework::DumpAllMultiCore;

        HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();
    }

    // --------------------------------------
    // Single Core env implementation
    // --------------------------------------

    FAST_CALL void CommitToLogPtrSingleCore(
        SmallTraceCyclicBuffer &buffer, const size_t trace_size, const char *src
    )
    {
        // 1. Allocate space for the trace
        if (hal::AtomicSub(&buffer.bytes_left, static_cast<i32>(trace_size)) < 0) {
            hal::AtomicAdd(&buffer.bytes_left, static_cast<i32>(trace_size));

            /* No space for trace coming from this interrupt, we are dropping it */
            return;
        }

        // 2. Adjust head ptr
        i32 old_head;
        i32 new_head;
        do {
            old_head = hal::AtomicLoad(&buffer.head);
            new_head = (old_head + static_cast<i32>(trace_size)) %
                       static_cast<i32>(SmallTraceCyclicBuffer::kSize);
        } while (hal::AtomicCompareExchange(&buffer.head, old_head, new_head) != old_head);

        // 3. Write the message
        if (new_head < old_head) {
            // We cross the boundary

            const i32 first_part  = static_cast<i32>(SmallTraceCyclicBuffer::kSize) - old_head;
            const i32 second_part = static_cast<i32>(trace_size) - first_part;
            memcpy(buffer.buffer + old_head, src, first_part);
            memcpy(buffer.buffer, src + first_part, second_part);
        } else {
            memcpy(buffer.buffer + old_head, src, static_cast<i32>(trace_size));
        }
    }

    char *GetWorkspaceSingleCore() { return single_core_env.main_execution_workspace; }

    void CommitToLogSingleCore(const size_t size)
    {
        CommitToLogPtrSingleCore(trace_log, size, GetWorkspaceSingleCore());
    }

    void CommitToDebugLogSingleCore(const size_t size)
    {
        CommitToLogPtrSingleCore(trace_debug_log, size, GetWorkspaceSingleCore());
    }

    void DumpAllSingleCore()
    {
        /* There is possibility to write out garbage at this stage but everything important is in
         * place */
        DumpBufferSingleCore<false>(trace_log);
        DumpBufferSingleCore<true>(trace_debug_log);
    }

    // -------------------------------------------------
    // Single core interrupts env implementation
    // -------------------------------------------------

    template <bool kIsDebug>
    FAST_CALL void HardenSingleCore(const char *src, const size_t size)
    {
        const char *start = src;
        const char *end   = src + size;

        while (start != end) {
            if (*start != '\0') {
                if constexpr (kIsDebug) {
                    hal::DebugTerminalPutChar(*start);
                } else {
                    hal::TerminalPutChar(*start);
                }
            }

            ++start;
        }
    }

    template <bool kIsDebug>
    FAST_CALL void DumpBufferSingleCore(SmallTraceCyclicBuffer &buffer)
    {
        // 1. Save exact amount of bytes to write at entry level (IMPORTANT: to not write infinite)
        const i32 bytes_left = hal::AtomicLoad(&buffer.bytes_left);
        i32 bytes_to_write   = static_cast<i32>(SmallTraceCyclicBuffer::kSize) - bytes_left;

        // 1. Write out the content
        while (bytes_to_write > 0) {
            // 1. Prepare old tail and write_size
            const i32 tail = buffer.tail.value;
            const i32 batch_write_size =
                std::min(bytes_to_write, static_cast<i32>(SmallTraceCyclicBuffer::kBatchSize));

            if (tail + batch_write_size > static_cast<i32>(SmallTraceCyclicBuffer::kSize)) {
                // We cross the boundary
                const size_t first_write  = SmallTraceCyclicBuffer::kSize - tail;
                const size_t second_write = batch_write_size - first_write;
                HardenSingleCore<kIsDebug>(buffer.buffer + tail, static_cast<size_t>(first_write));
                HardenSingleCore<kIsDebug>(buffer.buffer, static_cast<size_t>(second_write));
            } else {
                HardenSingleCore<kIsDebug>(
                    buffer.buffer + tail, static_cast<size_t>(batch_write_size)
                );
            }

            // 2. Push the tail
            hal::AtomicStore(
                &buffer.tail,
                (tail + batch_write_size) % static_cast<i32>(SmallTraceCyclicBuffer::kSize)
            );

            // 3. Release space
            hal::AtomicAdd(&buffer.bytes_left, batch_write_size);

            // 4. Iterate further
            bytes_to_write -= batch_write_size;
        }
    }

    FORCE_INLINE_F void CommitToLogPtrSingleCoreInterrupts(
        SmallTraceCyclicBuffer &buffer, const size_t trace_size
    )
    {
        const u8 nested_intrs = hardware::GetCoreLocalNestedInterrupts();
        const char *src       = nested_intrs == 0 ? single_core_env.main_execution_workspace
                                                  : single_core_env.interrupt_workspace[nested_intrs - 1];

        CommitToLogPtrSingleCore(buffer, trace_size, src);
    }

    char *GetWorkspaceSingleCoreInterrupts()
    {
        const u8 nested_intrs = hardware::GetCoreLocalNestedInterrupts();

        if (nested_intrs == 0) {
            return single_core_env.main_execution_workspace;
        }

        return single_core_env.interrupt_workspace[nested_intrs - 1];
    }

    void CommitToLogSingleCoreInterrupts(const size_t trace_size)
    {
        CommitToLogPtrSingleCoreInterrupts(trace_log, trace_size);
    }

    void CommitToDebugLogSingleCoreInterrupts(const size_t trace_size)
    {
        CommitToLogPtrSingleCoreInterrupts(trace_debug_log, trace_size);
    }

    void DumpAllSingleCoreInterrupts()
    {
        /* There is possibility to write out garbage at this stage but everything important is in
         * place */
        DumpBufferSingleCore<false>(trace_log);
        DumpBufferSingleCore<true>(trace_debug_log);
    }

    // ------------------------------------
    // Multicore env implementation
    // ------------------------------------

    char *GetWorkspaceMultiCore() { return nullptr; }

    void CommitToLogMultiCore(const size_t) {}

    void CommitToDebugLogMultiCore(const size_t) {}

    void DumpAllMultiCore() {}

    // ------------------------------
    // Global fields
    // ------------------------------

    TracingStage stage = TracingStage::kSingleCoreEnv;

    StageCallbacks stage_callbacks{
        .get_workspace_cb       = &TraceFramework::GetWorkspaceSingleCore,
        .commit_to_log_cb       = &TraceFramework::CommitToLogSingleCore,
        .commit_to_debug_log_cb = &TraceFramework::CommitToDebugLogSingleCore,
        .dump_all               = &TraceFramework::DumpAllSingleCore,
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

        /* Time module */
        rv[static_cast<size_t>(TraceModule::kTime)] =
            static_cast<TraceLevel>(FeatureValue<FeatureFlag::kTimeTraceLevel>);

        return rv;
    }();

    // ------------------------------
    // Single core env fields
    // ------------------------------

    CoreTraceData single_core_env{};
    SmallTraceCyclicBuffer trace_log{};
    SmallTraceCyclicBuffer trace_debug_log{};

    // ------------------------------
    // Multi core env fields
    // ------------------------------

    MultiCoreTraceCyclicBuffer dyn_trace_log{};
    MultiCoreTraceCyclicBuffer dyn_debug_trace_log{};
    CoreTraceData *multicore_env = nullptr;

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

int WriteTraceData(char *dst, TraceModule module, const TraceType type)
{
    ASSERT_NOT_NULL(dst);
    ASSERT_NEQ(type, TraceType::kShell);
    ASSERT_NEQ(type, TraceType::kLast);

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

    Sched::Pid pid{};
    if (hardware::GetCoreLocalTcb() != nullptr) {
        pid = hardware::GetRunningPid();
    }

    return snprintf(
        dst, FeatureValue<FeatureFlag::kSingleTraceMaxSize>,
        "[%s] "
        "[MOD:%s] "      // Module name
        "[TIME:%llu] "   // Timestamp
        "[CORE:%u] "     // Core ID
        "[PROC:%llu] ",  // Process ID
        type == TraceType::kKernelLog ? "LOG" : "DEBUG", module_name, system_time, core_id, pid
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

void DumpAllBuffersOnFailure() { (g_TraceFramework.*g_TraceFramework.stage_callbacks.dump_all)(); }

void TraceDumperTask() { (g_TraceFramework.*g_TraceFramework.stage_callbacks.dump_all)(); }

}  // namespace trace
