#ifndef ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_TPP_
#define ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_TPP_

#include <assert.h>
#include <autogen/feature_flags.h>
#include <stdio.h>

namespace internal
{
char *GetWorkspace();
void CommitToLog(size_t trace_size);
void CommitToDebugLog(size_t trace_size);
int WriteTraceData(char *dst, trace::TraceModule module, trace::TraceType type);
}  // namespace internal

namespace trace
{
template <TraceType type, TraceModule module, TraceLevel level, class... Args>
PREVENT_INLINE static void Write(const char *format, Args... args)
{
    static_assert(type != TraceType::kShell, "Shell output for traces is not yet supported...");
    static constexpr size_t kWorkspaceSize = FeatureValue<FeatureFlag::kSingleTraceMaxSize>;

    if constexpr (type == TraceType::kDebugOnly && !FeatureEnabled<FeatureFlag::kDebugTraces>) {
        return;
    }

    if (level > GetTraceLevel(module)) {
        return;
    }

    char *workspace = internal::GetWorkspace();

    /* Write kernel trace info */
    const int trace_info = internal::WriteTraceData(workspace, module, type);
    ASSERT_GE(trace_info, 0);
    const size_t trace_info_size = trace_info;

    /* User message */
    [[maybe_unused]] const u64 bytesWritten =
        snprintf(workspace + trace_info_size, kWorkspaceSize - trace_info_size, format, args...);
    ASSERT_LE(bytesWritten, kWorkspaceSize - trace_info_size);

    /* Ensure to write eol */
    bool extended = false;
    if (bytesWritten + trace_info_size == kWorkspaceSize - 1) {
        /* write to last char */
        workspace[kWorkspaceSize - 2] = '\n';
    } else {
        workspace[bytesWritten + trace_info_size]     = '\n';
        workspace[bytesWritten + trace_info_size + 1] = '\0';
        extended                                      = true;
    }

    if constexpr (type == TraceType::kKernelLog) {
        internal::CommitToLog(bytesWritten + trace_info_size + 1 + extended);
    }

    if constexpr (type == TraceType::kDebugOnly) {
        internal::CommitToDebugLog(bytesWritten + trace_info_size + 1 + extended);
    }
}
}  // namespace  trace
#endif  // ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_TPP_
