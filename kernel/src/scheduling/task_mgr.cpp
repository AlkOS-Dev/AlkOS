#include <template/scope_guard.hpp>

#include "modules/memory.hpp"
#include "modules/scheduling.hpp"
#include "scheduling/kworker.hpp"
#include "task_mgr.hpp"
#include "trace_framework.hpp"

#include "hal/scheduling.hpp"

// ------------------------------
// statics
// ------------------------------

// ------------------------------
// Implementations
// ------------------------------

namespace Sched
{
void TaskMgr::InitializeMultitasking()
{
    // Spawn 3 Kernel Workers
    static constexpr size_t kNumKWorkers = 0;
    for (size_t i = 0; i < kNumKWorkers; ++i) {
        auto result = SpawnProcess(KWorkerMain, true);
        R_ASSERT_TRUE(
            static_cast<bool>(result),
            "Failed to spawn kernel workers. Not enough resources for the system"
        );

        TRACE_INFO_SCHEDULING(
            "Created initial Kernel Worker process with Pid: %llu", result.value()
        );
    }
}

std::expected<std::tuple<Pid, Tid>, Error> TaskMgr::SpawnProcess(
    void (*f)(), const bool kernel_only
)
{
    bool dismiss = false;

    if (kernel_only) {
        ASSERT_TRUE(hal::IsKernelSpace(reinterpret_cast<void *>(f)));
    } else {
        ASSERT_FALSE(hal::IsKernelSpace(reinterpret_cast<void *>(f)));
    }

    // 1. Prepare internal structure for the process
    auto process = SchedulingModule::Get().GetProcesses().PrepareProcess();
    if (!process) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create process. Failed on struct allocation: %s", to_string(process.error())
        );
        return std::unexpected(process.error());
    }
    template_lib::BatchedScopeGuard process_guard(dismiss, [&]() {
        [[maybe_unused]] const auto result =
            SchedulingModule::Get().GetProcesses().Free(process.value()->pid);
        ASSERT_TRUE(static_cast<bool>(result));
    });

    // 2. Fill process data and resources - TODO: replace with proper one
    if (kernel_only) {
        process.value()->address_space         = &MemoryModule::Get().GetKernelAddressSpace();
        process.value()->flags.KernelSpaceOnly = true;
    } else {
        R_FAIL_ALWAYS("User space tasks not supported!");

        TODO_WHEN_VMEM_WORKS
        // auto addr_space = MemoryModule::Get().GetVmm().CreateAddrSpace();
        // if (!addr_space) {
        //     DEBUG_WARN_SCHEDULING(
        //         "Failed to create process. Failed on AddressSpace creation: %s",
        //         template_lib::to_string(addr_space.error()).data()
        //     );
        //     return std::unexpected(Error::OutOfMemory);
        // }
        // process.value()->address_space = addr_space.value();
        // template_lib::BatchedScopeGuard addr_space_guard(dismiss, [&]() {
        //     [[maybe_unused]] const auto result =
        //         MemoryModule::Get().GetVmm().DestroyAddrSpace(addr_space.value());
        //     ASSERT_TRUE(static_cast<bool>(result));
        // });
    }

    // 3. Spawn first thread
    const auto tid = SpawnThread(process.value()->pid, f);
    if (!tid) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create process. Failed on initial thread creation: %s",
            to_string(tid.error())
        );
        return std::unexpected(tid.error());
    }

    // 4. Add to scheduler - TODO

    DEBUG_INFO_SCHEDULING(
        "Created process with pid: %llu, and initial thread with tid: %llu", process.value()->pid,
        tid.value()
    );

    dismiss = true;
    return std::make_tuple(process.value()->pid, tid.value());
}

std::expected<Tid, Error> TaskMgr::SpawnThread(const Pid pid, void (*f)())
{
    bool dismiss = false;

    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    ASSERT_TRUE(static_cast<bool>(process));

    // 1. Prepare internal structure for execution unit - thread:
    auto thread = SchedulingModule::Get().GetThreads().PrepareThread();
    if (!thread) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create thread for %llu. Failed on struct allocation: %s", pid,
            to_string(thread.error())
        );

        return std::unexpected(thread.error());
    }
    template_lib::BatchedScopeGuard thread_guard(dismiss, [&]() {
        [[maybe_unused]] const auto result =
            SchedulingModule::Get().GetThreads().Free(thread.value()->tid);
        ASSERT_TRUE(static_cast<bool>(result));
    });

    // 2. Allocate process resources and prepare the struct
    thread.value()->owner = pid;

    // 2.1 Kernel Stack
    const auto kernel_stack = Mem::KMallocAligned({kKernelStackSize, kStackAlignment});
    if (!kernel_stack) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create thread for %llu. Failed on kernel stack allocation: %s", pid,
            to_string(kernel_stack.error())
        );

        return std::unexpected(Error::OutOfMemory);
    }
    template_lib::BatchedScopeGuard esp0_guard(dismiss, [&]() {
        Mem::KFreeAligned(kernel_stack.value());
    });
    thread.value()->kernel_stack        = kernel_stack.value();
    thread.value()->kernel_stack_bottom = kernel_stack.value();

    // 2.2 Thread Stack
    if (!process.value()->flags.KernelSpaceOnly) {
        const auto thread_stack = Mem::KMallocAligned({kStackSize, kStackAlignment});
        if (!thread_stack) {
            DEBUG_WARN_SCHEDULING(
                "Failed to create thread for %llu. Failed on thread stack allocation: %s", pid,
                to_string(thread_stack.error())
            );

            return std::unexpected(Error::OutOfMemory);
        }
        thread.value()->user_stack        = thread_stack.value();
        thread.value()->user_stack_bottom = thread_stack.value();
    } else {
        thread.value()->user_stack        = nullptr;
        thread.value()->user_stack_bottom = nullptr;
    }

    if (process.value()->flags.KernelSpaceOnly) {
        hal::InitializeStackKThread(&thread.value()->kernel_stack, f);
    } else {
        hal::InitializeStackUserThread(&thread.value()->kernel_stack, f);
    }

    dismiss = true;
    return thread.value()->tid;
}
}  // namespace Sched
