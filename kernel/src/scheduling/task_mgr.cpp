#include <template/enum_name.hpp>
#include <template/scope_guard.hpp>

#include "modules/memory.hpp"
#include "modules/scheduling.hpp"
#include "task_mgr.hpp"
#include "trace_framework.hpp"

// ------------------------------
// statics
// ------------------------------

// ------------------------------
// Implementations
// ------------------------------

namespace Sched
{
void TaskMgr::InitializeMultitasking() {}

std::expected<Pid, Error> TaskMgr::SpawnProcess()
{
    bool dismiss = false;

    // 1. Prepare internal structure for the process
    auto process = SchedulingModule::Get().GetProcesses().PrepareProcess();
    if (!process) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create process. Failed on struct allocation: %s",
            template_lib::to_string(process.error()).data()
        );
        return std::unexpected(process.error());
    }
    template_lib::BatchedScopeGuard process_guard(dismiss, [&]() {
        [[maybe_unused]] const auto result =
            SchedulingModule::Get().GetProcesses().Free(process.value()->pid);
        ASSERT_TRUE(static_cast<bool>(result));
    });

    // 2. Fill process data and resources
    auto addr_space = MemoryModule::Get().GetVmm().CreateAddrSpace();
    if (!addr_space) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create process. Failed on AddressSpace creation: %s",
            template_lib::to_string(addr_space.error()).data()
        );
        return std::unexpected(Error::OutOfMemory);
    }
    process.value()->address_space = addr_space.value();
    template_lib::BatchedScopeGuard addr_space_guard(dismiss, [&]() {
        [[maybe_unused]] const auto result =
            MemoryModule::Get().GetVmm().DestroyAddrSpace(addr_space.value());
        ASSERT_TRUE(static_cast<bool>(result));
    });

    // 3. Spawn first thread
    const auto tid = SpawnThread(process.value()->pid);
    if (!tid) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create process. Failed on initial thread creation: %s",
            template_lib::to_string(tid.error()).data()
        );
        return std::unexpected(tid.error());
    }

    // 4. Add to scheduler - TODO

    DEBUG_INFO_SCHEDULING(
        "Created process with pid: %llu, and initial thread with tid: %llu", process.value()->pid,
        tid.value()
    );

    dismiss = true;
    return process.value()->pid;
}

std::expected<Tid, Error> TaskMgr::SpawnThread(const Pid pid)
{
    bool dismiss = false;

    // 2. Prepare internal structure for execution unit - thread:
    auto thread = SchedulingModule::Get().GetThreads().PrepareThread();
    if (!thread) {
        return std::unexpected(thread.error());
    }
    template_lib::BatchedScopeGuard thread_guard(dismiss, [&]() {
        [[maybe_unused]] const auto result =
            SchedulingModule::Get().GetThreads().Free(thread.value()->tid);
        ASSERT_TRUE(static_cast<bool>(result));
    });

    // 3. Allocate process resources and prepare the struct
    thread.value()->owner = pid;

    // 3.1 Kernel Stack
    const auto kernel_stack = Mem::KMallocAligned({kKernelStackSize, kStackAlignment});
    if (!kernel_stack) {
        return std::unexpected(Error::OutOfMemory);
    }
    template_lib::BatchedScopeGuard esp0_guard(dismiss, [&]() {
        Mem::KFreeAligned(kernel_stack.value());
    });
    thread.value()->kernel_stack = kernel_stack.value();

    // 3.2 Thread Stack
    const auto thread_stack = Mem::KMallocAligned({kStackSize, kStackAlignment});
    if (!thread_stack) {
        return std::unexpected(Error::OutOfMemory);
    }
    thread.value()->user_stack = thread_stack.value();

    dismiss = true;
    return thread.value()->tid;
}
}  // namespace Sched
