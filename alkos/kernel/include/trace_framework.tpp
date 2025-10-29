#ifndef ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_TPP_
#define ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_TPP_

#include <assert.h>
#include <autogen/feature_flags.h>
#include <stdio.h>

namespace internal
{
char* GetWorkspace();
void CommitToLog(size_t trace_size);
void CommitToDebugLog(size_t trace_size);
size_t WriteTraceData(char* dst, trace::TraceModule module);
}  // namespace internal

namespace trace
{
template <TraceType type, TraceModule module, TraceLevel level, class... Args>
FAST_CALL void Write(const char* format, Args... args)
{
    static_assert(type != TraceType::kShell, "Shell output for traces is not yet supported...");
    static constexpr size_t kWorkspaceSize = FeatureValue<FeatureFlag::kSingleTraceMaxSize>;

    if constexpr (type == TraceType::kDebugOnly && !FeatureEnabled<FeatureFlag::kDebugTraces>) {
        return;
    }

    if (level > GetTraceLevel(module)) {
        return;
    }

    char* workspace = internal::GetWorkspace();

    /* Write kernel trace info */
    const size_t trace_info_size = internal::WriteTraceData(workspace, module);

    /* User message */
    [[maybe_unused]] const u64 bytesWritten =
        snprintf(workspace + trace_info_size, kWorkspaceSize - trace_info_size, format, args...);
    ASSERT_LT(bytesWritten, kWorkspaceSize - trace_info_size);

    if constexpr (type == TraceType::kKernelLog) {
        internal::CommitToLog(bytesWritten);
    }

    if constexpr (type == TraceType::kDebugOnly) {
        internal::CommitToDebugLog(bytesWritten);
    }
}
}  // namespace  trace
#endif  // ALKOS_KERNEL_INCLUDE_TRACE_FRAMEWORK_TPP_
