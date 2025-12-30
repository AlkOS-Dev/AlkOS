#include <template/scope_guard.hpp>

#include "constants.hpp"
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
    static constexpr size_t kNumKWorkers = 3;
    for (size_t i = 0; i < kNumKWorkers; ++i) {
        char name[] = "kworker-0";

        name[sizeof(name) - 2] = static_cast<char>('0' + i);
        auto result            = SpawnKernelProcess(name, {}, KWorkerMain);
        R_ASSERT_TRUE(
            static_cast<bool>(result),
            "Failed to spawn kernel workers. Not enough resources for the system"
        );

        TRACE_INFO_SCHEDULING(
            "Created initial Kernel Worker process with Pid: %llu", result.value().get<0>()
        );
    }

    // Spawn trace dumper
    const auto result = SpawnKernelProcess("kworker-trace-dumper", {}, TraceDumperMain);
    R_ASSERT_TRUE(static_cast<bool>(result), "Failed to spawn trace dumper process...");
}

std::expected<Pid, Error> TaskMgr::SpawnEmptyProcess(const char *name, const ProcessFlags flags)
{
    if (strnlen(name, Process::kMaxNameLength) == Process::kMaxNameLength) {
        return std::unexpected(Error::ProcessNameTooLong);
    }

    // 1. Prepare internal structure for the process
    auto process = SchedulingModule::Get().GetProcesses().PrepareProcess();
    if (!process) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create process. Failed on struct allocation: %s", to_string(process.error())
        );
        return std::unexpected(process.error());
    }
    template_lib::ScopeGuard process_guard([&] {
        [[maybe_unused]] const auto result =
            SchedulingModule::Get().GetProcesses().Free(process.value()->pid);
        ASSERT_TRUE(static_cast<bool>(result));
    });

    // 1.1 Fill process properties
    process.value()->flags = flags;
    strcpy(process.value()->name, name);

    // 2. Prepare address space
    Mem::AddressSpace *address_space{};
    if (flags.KernelSpaceOnly) {
        address_space = &MemoryModule::Get().GetKernelAddressSpace();
    } else {
        auto as = MemoryModule::Get().GetVmm().CreateUserAddrSpace();
        if (!as) {
            DEBUG_WARN_SCHEDULING(
                "Failed to create process. Failed on AddressSpace creation: %s",
                to_string(as.error())
            );
            return std::unexpected(Error::OutOfMemory);
        }
        address_space = as.value();
    }

    process.value()->address_space = address_space;
    process_guard.dismiss();

    return process.value()->pid;
}

std::expected<Thread *, Error> TaskMgr::SpawnThread(const Pid pid, void (*f)())
{
    bool dismiss = false;

    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    ASSERT_TRUE(static_cast<bool>(process));

    if (process.value()->flags.KernelSpaceOnly) {
        ASSERT_TRUE(IsKernelSpace(reinterpret_cast<void *>(f)));
    } else {
        ASSERT_FALSE(IsKernelSpace(reinterpret_cast<void *>(f)));
    }

    // 1. Prepare internal structure for execution unit - thread:
    auto thread = SchedulingModule::Get().GetThreads().PrepareThread();
    if (!thread) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create thread for %llu. Failed on struct allocation: %s", pid,
            to_string(thread.error())
        );

        return std::unexpected(thread.error());
    }
    template_lib::BatchedScopeGuard thread_guard(dismiss, [&] {
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
    template_lib::BatchedScopeGuard esp0_guard(dismiss, [&] {
        Mem::KFreeAligned(kernel_stack.value());
    });
    thread.value()->kernel_stack        = kernel_stack.value();
    thread.value()->kernel_stack_bottom = kernel_stack.value();

    // 2.2 Thread Stack
    if (!process.value()->flags.KernelSpaceOnly) {
        const auto thread_stack =
            MemoryModule::Get().GetVmm().AllocUserStack(process.value()->address_space, kStackSize);
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
    return thread.value();
}

std::expected<std::tuple<Pid, Tid>, Error> TaskMgr::SpawnKernelProcess(
    const char *name, ProcessFlags flags, void (*f)()
)
{
    flags.KernelSpaceOnly = true;
    ASSERT_TRUE(IsKernelSpace(reinterpret_cast<void *>(f)));

    auto process = SpawnEmptyProcess(name, flags);
    if (!process) {
        return std::unexpected(process.error());
    }

    template_lib::ScopeGuard process_guard([&] {
        const auto result = SchedulingModule::Get().GetProcesses().Free(process.value());
        ASSERT_TRUE(static_cast<bool>(result));
    });

    const auto thread = SpawnThread(process.value(), f);
    if (!thread) {
        return std::unexpected(thread.error());
    }

    HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
    SchedulingModule::Get().GetScheduler().AddReadyThread(thread.value());
    HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();

    DEBUG_INFO_SCHEDULING(
        "Created kernel process with pid: %llu, name: %s and initial thread with tid: %llu",
        process.value(), name, thread.value()->tid
    );

    process_guard.dismiss();
    return std::make_tuple(process.value(), thread.value()->tid);
}

std::expected<Pid, Error> TaskMgr::CloneProcess(Pid) { R_FAIL_ALWAYS("NOT IMPLEMENTED"); }

std::expected<Tid, Error> TaskMgr::ExecuteElf64(Pid, const char *)
{
    R_FAIL_ALWAYS("NOT IMPLEMENTED");
}

std::expected<std::tuple<Pid, Tid>, Error> TaskMgr::ExecuteElf64(
    const char *path, const ProcessFlags flags
)
{
    auto process = SpawnEmptyProcess(path, flags);
    if (!process) {
        return std::unexpected(process.error());
    }

    template_lib::ScopeGuard process_guard([&] {
        const auto result = SchedulingModule::Get().GetProcesses().Free(process.value());
        ASSERT_TRUE(static_cast<bool>(result));
    });

    auto thread = ExecuteElf64(process.value(), path);
    if (!thread) {
        return std::unexpected(thread.error());
    }

    process_guard.dismiss();
    return std::make_tuple(process.value(), thread.value());
}

std::expected<void, Error> TaskMgr::KillProcess(Pid pid) { R_FAIL_ALWAYS("NOT IMPLEMENTED"); }

std::expected<void, Error> TaskMgr::ExitProcess(Pid pid) { R_FAIL_ALWAYS("NOT IMPLEMENTED"); }

std::expected<void, Error> TaskMgr::ExitThread(Tid tid) { R_FAIL_ALWAYS("NOT IMPLEMENTED"); }

}  // namespace Sched
