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
        static constexpr size_t kSize           = 65536;
        static constexpr size_t kDumpSize       = kSize / 4;
        static constexpr size_t kBatchSize      = 512;
        static constexpr size_t kBufferDumpDone = kSize - kSize / 8;

        hal::Atomic32 bytes_left{.value = kSize};
        hal::Atomic32 head{};
        hal::Atomic32 tail{};
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

    FORCE_INLINE_F void AdvanceToMultiThreadStage() { R_FAIL_ALWAYS("Not implemented"); }

    // --------------------------------------
    // Single thread env implementation
    // --------------------------------------

    char *GetWorkspaceSingleThread() { return single_thread_env.main_execution_workspace; }

    void CommitToLogSingleThread(const size_t)
    {
        /* Bypass kernel log as we are only writers and we do not care bout perf here */
        HardenFullString(single_thread_env.main_execution_workspace);
    }

    void CommitToDebugLogSingleThread(const size_t)
    {
        hal::DebugTerminalWrite(single_thread_env.main_execution_workspace);
    }

    // -------------------------------------------------
    // Single thread interrupts env implementation
    // -------------------------------------------------

    template <bool kIsDebug>
    FAST_CALL void HardenSingleThread(const char *src, const size_t size)
    {
        const char *start = src;
        const char *end   = src + size;

        while (start != end) {
            if constexpr (kIsDebug) {
                hal::DebugTerminalPutChar(*src);
            } else {
                hal::TerminalPutChar(*src);
            }
            ++start;
        }
    }

    template <bool kIsDebug>
    FORCE_INLINE_F void DumpBufferSingleThread(SmallTraceCyclicBuffer &buffer)
    {
        // 1. Save exact amount of bytes to write at entry level (IMPORTANT: to not write infinite)
        const i32 bytes_left = hal::AtomicIncrement(&buffer.bytes_left);
        i32 bytes_to_write   = static_cast<i32>(SmallTraceCyclicBuffer::kSize) - bytes_left;

        while (bytes_to_write > 0) {
            // 1. Write out the content
            const i32 tail = buffer.tail.value;
            const i32 batch_write_size =
                std::min(bytes_to_write, static_cast<i32>(SmallTraceCyclicBuffer::kBatchSize));

            if (tail + batch_write_size > SmallTraceCyclicBuffer::kSize) {
                // We cross the boundary

            } else {
                HardenSingleThread<kIsDebug>(
                    buffer.buffer + tail, static_cast<size_t>(bytes_to_write)
                );
            }

            // 2. Push the tail
            buffer.tail.value = (buffer.tail.value + batch_write_size) %
                                static_cast<i32>(SmallTraceCyclicBuffer::kSize);

            // 3. Release space
            hal::AtomicAdd(&buffer.bytes_left, batch_write_size);

            // 4. Iterate further
            bytes_to_write -= batch_write_size;
        }
    }

    template <bool kIsDebug>
    FORCE_INLINE_F void CommitToLogPtrSingleThreadInterrupts(
        SmallTraceCyclicBuffer &buffer, const size_t trace_size
    )
    {
        const u8 nested_intrs = hardware::GetCoreLocalData().nested_interrupts;
        i32 available_bytes;
        i32 bytes_left;
        i32 new_value;

        // 1. Allocate space for the trait
        do {
            do {
                /* For main execution just empty the space and retry */
                bytes_left = hal::AtomicLoad(&buffer.bytes_left);
                DumpBufferSingleThread<kIsDebug>(buffer);
            } while (nested_intrs == 0 && bytes_left < trace_size);

            /* For interrupt traces, some information may be lost */

            available_bytes = std::min(bytes_left, static_cast<i32>(trace_size));
            new_value       = bytes_left - available_bytes;
        } while (hal::AtomicCompareExchange(&buffer.bytes_left, bytes_left, new_value) !=
                 bytes_left);

        char *src = nested_intrs == 0 ? single_thread_env.main_execution_workspace
                                      : single_thread_env.interrupt_workspace[nested_intrs - 1];
        if (nested_intrs != 0) {
            if (available_bytes == 0) {
                return;
            }

            /* Ensure EOS for interrupt traces */
            src[available_bytes - 1] = '\0';
        }

        // 2. Adjust head ptr
        i32 old_head;
        i32 new_head;
        do {
            old_head = hal::AtomicLoad(&buffer.head);
            new_head =
                (old_head + available_bytes) % static_cast<i32>(SmallTraceCyclicBuffer::kSize);
        } while (hal::AtomicCompareExchange(&buffer.head, old_head, new_head) != old_head);

        // 3. Write the message
        if (new_head < old_head) {
            // our message is not continuous
            const i32 first_part  = static_cast<i32>(SmallTraceCyclicBuffer::kSize) - old_head;
            const i32 second_part = new_head;

            strncpy(buffer.buffer + old_head, src, first_part);
            strncpy(buffer.buffer, src + first_part, second_part);
        } else {
            strcpy(buffer.buffer + old_head, src);
        }

        if (nested_intrs == 0 && buffer.bytes_left.value < SmallTraceCyclicBuffer::kDumpSize) {
            // Dump the content
            DumpBufferSingleThread<kIsDebug>(buffer);
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
        CommitToLogPtrSingleThreadInterrupts<false>(trace_log, trace_size);
    }

    void CommitToDebugLogSingleThreadInterrupts(const size_t trace_size)
    {
        CommitToLogPtrSingleThreadInterrupts<true>(trace_debug_log, trace_size);
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
