#include <template/scope_guard.hpp>

#include "constants.hpp"
#include "modules/memory.hpp"
#include "modules/scheduling.hpp"
#include "modules/vfs.hpp"
#include "modules/video.hpp"
#include "scheduling/kworker.hpp"
#include "task_mgr.hpp"

#include "hal/scheduling.hpp"
#include "modules/timing.hpp"
#include "scheduling/local_lock.hpp"
#include "trace_framework.hpp"

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
    SchedulingModule::Get().GetScheduler().InstallInterruptHandler();

    // Spawn trace dumper
    const auto result0 =
        SpawnKernelProcess("kworker-trace-dumper", {}, PrepareKThreadTask(TraceDumperMain));
    R_ASSERT_TRUE(static_cast<bool>(result0), "Failed to spawn trace dumper process...");

    // Spawn thread ripper
    const auto result1 =
        SpawnKernelProcess("kworker-thread-ripper", {}, PrepareKThreadTask(ThreadRipperMain));
    R_ASSERT_TRUE(static_cast<bool>(result1), "Failed to spawn thread ripper...");

    const auto result2 =
        SpawnKernelProcess("kworker-process-ripper", {}, PrepareKThreadTask(ProcessRipperMain));
    R_ASSERT_TRUE(static_cast<bool>(result2), "Failed to spawn process ripper...");
}

std::expected<Pid, Error> TaskMgr::SpawnEmptyProcess(const char *name, const ProcessFlags flags)
{
    LocalCoreLock local_lock{};

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
    process_guard.Dismiss();

    return process.value()->pid;
}

std::expected<Thread *, Error> TaskMgr::SpawnThread(const Pid pid, const Task &task)
{
    LocalCoreLock local_lock{};
    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    ASSERT_TRUE(static_cast<bool>(process));

    ThreadFlags flags{};

    flags.preserve_floats = process.value()->flags.PreserveFloats;
    flags.policy          = SchedulingPolicy::kNormalTasks_MLFQ_P3;
    flags.priority        = 0;
    flags.user_priority   = UserPriority::kMedium;

    return SpawnThread(process.value(), flags, task);
}

std::expected<Thread *, Error> TaskMgr::SpawnThread(
    const Pid pid, const ThreadFlags flags, const Task &task
)
{
    LocalCoreLock local_lock{};

    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    ASSERT_TRUE(static_cast<bool>(process));

    return SpawnThread(process.value(), flags, task);
}

std::expected<Thread *, Error> TaskMgr::SpawnThread(
    Process *process, const ThreadFlags flags, const Task &task
)
{
    LocalCoreLock local_lock{};

    bool dismiss = false;

    // 1. Prepare internal structure for execution unit - thread:
    auto thread = SchedulingModule::Get().GetThreads().PrepareThread();
    if (!thread) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create thread for %llu. Failed on struct allocation: %s", process->pid,
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
    thread.value()->owner = process->pid;
    thread.value()->flags = flags;
    thread.value()->state = ThreadState::kReady;

    // 2.1 Kernel Stack
    const auto kernel_stack = Mem::KMallocAligned({kKernelStackSize, kStackAlignment});
    if (!kernel_stack) {
        DEBUG_WARN_SCHEDULING(
            "Failed to create thread for %llu. Failed on kernel stack allocation: %s", process->pid,
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
    if (!process->flags.KernelSpaceOnly) {
        const auto thread_stack =
            MemoryModule::Get().GetVmm().AllocUserStack(process->address_space, kStackSize);
        if (!thread_stack) {
            DEBUG_WARN_SCHEDULING(
                "Failed to create thread for %llu. Failed on thread stack allocation: %s",
                process->pid, to_string(thread_stack.error())
            );

            return std::unexpected(Error::OutOfMemory);
        }
        thread.value()->user_stack        = thread_stack.value();
        thread.value()->user_stack_bottom = thread_stack.value();
    } else {
        thread.value()->user_stack        = nullptr;
        thread.value()->user_stack_bottom = nullptr;
    }

    hal::InitializeThreadStack(&thread.value()->kernel_stack, task);

    process->live_threads++;
    data_structures::FronIntrusiveDoubleListView<Thread, kProcessListIntrusiveLevel>(
        process->threads
    )
        .PushFront(thread.value());

    dismiss = true;
    return thread.value();
}

std::expected<std::tuple<Pid, Tid>, Error> TaskMgr::SpawnKernelProcess(
    const char *name, ProcessFlags flags, const Task &task
)
{
    LocalCoreLock local_lock{};

    flags.KernelSpaceOnly = true;

    auto process = SpawnEmptyProcess(name, flags);
    if (!process) {
        return std::unexpected(process.error());
    }

    template_lib::ScopeGuard process_guard([&] {
        const auto result = SchedulingModule::Get().GetProcesses().Free(process.value());
        ASSERT_TRUE(static_cast<bool>(result));
    });

    const auto thread = SpawnThread(process.value(), task);
    if (!thread) {
        return std::unexpected(thread.error());
    }

    SchedulingModule::Get().GetScheduler().AddReadyThread(thread.value());

    DEBUG_INFO_SCHEDULING(
        "Created kernel process with pid: %llu, name: %s and initial thread with tid: %llu",
        process.value(), name, thread.value()->tid
    );

    process_guard.Dismiss();
    return std::make_tuple(process.value(), thread.value()->tid);
}

std::expected<Pid, Error> TaskMgr::CloneProcess(Pid) { R_FAIL_ALWAYS("NOT IMPLEMENTED"); }

std::expected<Tid, Error> TaskMgr::ExecuteElf64(const Pid pid, const char *path)
{
    LocalCoreLock local_lock{};

    auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    RET_UNEXPECTED_IF_ERR(process);

    template_lib::ScopeGuard process_guard([&] {
        const auto result = SchedulingModule::Get().GetProcesses().Free(process.value()->pid);
        ASSERT_TRUE(static_cast<bool>(result));
    });

    // TODO: KILL ALL EXISTING THREADS FROM THIS PROCESS

    const size_t path_size = strlen(path);
    const auto mem         = Mem::KMalloc(path_size);
    if (!mem) {
        return std::unexpected(Error::OutOfMemory);
    }
    memcpy(mem.value(), path, path_size);

    auto thread =
        SpawnThread(pid, PrepareElf64LoaderTask(pid, static_cast<const char *>(mem.value())));
    RET_UNEXPECTED_IF_ERR(thread);

    SchedulingModule::Get().GetScheduler().AddReadyThread(thread.value());

    DEBUG_INFO_SCHEDULING(
        "Created userspace process with pid: %llu and initial thread with tid: %llu", pid,
        thread.value()->tid
    );

    process_guard.Dismiss();
    return thread.value()->tid;
}

std::expected<std::tuple<Pid, Tid>, Error> TaskMgr::ExecuteElf64(
    const char *path, const ProcessFlags flags
)
{
    LocalCoreLock local_lock{};

    const auto exists_res = VfsModule::Get().FileExists(vfs::Path(path));
    if (!exists_res.has_value() || !exists_res.value()) {
        return std::unexpected(Error::ExecPathNotFound);
    }

    auto process = SpawnEmptyProcess(vfs::Path(path).GetFilename().data(), flags);
    RET_UNEXPECTED_IF_ERR(process);

    template_lib::ScopeGuard process_guard([&] {
        const auto result = SchedulingModule::Get().GetProcesses().Free(process.value());
        ASSERT_TRUE(static_cast<bool>(result));
    });
    auto thread = ExecuteElf64(process.value(), path);
    RET_UNEXPECTED_IF_ERR(thread);

    process_guard.Dismiss();
    return std::make_tuple(process.value(), thread.value());
}

std::expected<void, Error> TaskMgr::CommitMurder(const Tid tid)
{
    if (tid == hardware::GetCoreLocalTcb()->tid) {
        ThreadExit(nullptr);
    }

    LocalCoreLock local_lock{};

    DEBUG_INFO_SCHEDULING(
        "Thread with TID: %llu commiting murder on TID: %llu", hardware::GetCoreLocalTcb()->tid, tid
    );

    const auto thread = SchedulingModule::Get().GetThreads().GetThread(tid);
    RET_UNEXPECTED_IF_ERR(thread);

    if (thread.value()->state == ThreadState::kTerminated) {
        return {};
    }

    // 1. Remove from process thread list
    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(thread.value()->owner);
    RET_UNEXPECTED_IF_ERR(process);

    data_structures::FronIntrusiveDoubleListView<Thread, kProcessListIntrusiveLevel>(
        process.value()->threads
    )
        .Remove(thread.value());

    // 2. Remove thread from scheduler
    if (thread.value()->state != ThreadState::kWaitingForJoin) {
        SchedulingModule::Get().GetScheduler().RemoveThread(thread.value());
    }

    // 3. Mark for removal
    thread.value()->state = ThreadState::kTerminated;
    threads_to_clean_.Push(thread.value()->tid.id);
    process.value()->threads_to_clean++;
    process.value()->live_threads--;

    return {};
}

std::expected<void, Error> TaskMgr::CommitMurder(const Pid pid)
{
    if (hardware::GetRunningPid() == pid) {
        ExitProcess(-2);
    }

    LocalCoreLock local_lock{};
    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    RET_UNEXPECTED_IF_ERR(process);

    DEBUG_INFO_SCHEDULING(
        "Process with PID: %llu is commiting murder on PID: %llu", hardware::GetRunningPid(), pid
    );

    // 1. Kill all running threads
    data_structures::FronIntrusiveDoubleListView<Thread, kProcessListIntrusiveLevel> threads(
        process.value()->threads
    );
    while (!threads.IsEmpty()) {
        const auto thread = threads.PopFront();
        const auto result = CommitMurder(thread->tid);
        ASSERT_TRUE(static_cast<bool>(result));
    }

    // 2. Mark target as terminated
    process.value()->state  = ProcessState::kTerminated;
    process.value()->status = -2;
    processes_to_clean_.Push(process.value()->pid.id);

    return {};
}

void TaskMgr::CommitSuicide()
{
    // TODO: replace with urgent action
    ExitProcess(-1);

    const auto process =
        SchedulingModule::Get().GetProcesses().GetProcess(hardware::GetCoreLocalTcb()->owner);
    ASSERT_TRUE(static_cast<bool>(process));

    process.value()->state = ProcessState::kTerminated;
    processes_to_clean_.Push(process.value()->pid.id);
}

void TaskMgr::ExitProcess(const int status)
{
    LocalCoreLock lock{};

    const auto process =
        SchedulingModule::Get().GetProcesses().GetProcess(hardware::GetCoreLocalTcb()->owner);
    ASSERT_TRUE(static_cast<bool>(process));

    DEBUG_INFO_SCHEDULING("Process with PID: %llu exiting...", process.value()->pid);

    // 1. Kill all running threads except us
    data_structures::FronIntrusiveDoubleListView<Thread, kProcessListIntrusiveLevel> threads(
        process.value()->threads
    );
    while (!threads.IsEmpty()) {
        const auto thread = threads.PopFront();

        if (thread == hardware::GetCoreLocalTcb()) {
            continue;
        }

        const auto result = CommitMurder(thread->tid);
        ASSERT_TRUE(static_cast<bool>(result));
    }
    threads.PushFront(hardware::GetCoreLocalTcb());

    // 2. Mark self as waiting
    process.value()->state  = ProcessState::kWaitingForJoin;
    process.value()->status = status;

    ThreadExit(nullptr);
}

std::expected<Tid, Error> TaskMgr::CreateThread(const ThreadFlags flags, const Task &task)
{
    LocalCoreLock lock{};
    const auto thread = SpawnThread(hardware::GetRunningPid(), flags, task);

    if (!thread) {
        return std::unexpected(thread.error());
    }

    SchedulingModule::Get().GetScheduler().AddReadyThread(thread.value());
    return thread.value()->tid;
}

std::expected<Tid, Error> TaskMgr::CreateUserThread(
    const ThreadFlags flags, const thread_func_t func, void *arg
)
{
    const Task task = PrepareUserThreadTask(func, arg);
    return CreateThread(flags, task);
}

std::expected<void, Error> TaskMgr::DetachThread(const Tid tid)
{
    LocalCoreLock core_lock{};
    auto thread = SchedulingModule::Get().GetThreads().GetThread(tid);

    if (!thread) {
        return std::unexpected(thread.error());
    }

    if (thread.value()->state == ThreadState::kWaitingForJoin) {
        thread.value()->state = ThreadState::kTerminated;
        threads_to_clean_.Push(thread.value()->tid.id);

        auto process = SchedulingModule::Get().GetProcesses().GetProcess(thread.value()->owner);
        ASSERT_TRUE(static_cast<bool>(process));
        process.value()->threads_to_clean++;
        process.value()->live_threads--;

        return {};
    }

    thread.value()->flags.detached = true;
    return {};
}

void TaskMgr::ThreadExit(void *retval)
{
    const auto tcb = hardware::GetCoreLocalTcb();
    ASSERT_NOT_NULL(tcb);
    ASSERT_EQ(tcb->state, ThreadState::kRunning);

    DEBUG_INFO_SCHEDULING("Thread with tid: %llu exiting...", tcb->tid);

    tcb->retval = retval;

    LocalCoreLock core_lock{};

    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(tcb->owner);
    ASSERT_TRUE(static_cast<bool>(process));
    data_structures::FronIntrusiveDoubleListView<Thread, kProcessListIntrusiveLevel>(
        process.value()->threads
    )
        .Remove(tcb);

    ThreadState state{};
    if (tcb->flags.detached || process.value()->live_threads == 1) {
        ASSERT_NOT_ZERO(process.value()->live_threads);

        state = ThreadState::kTerminated;
        threads_to_clean_.Push(tcb->tid.id);
        process.value()->threads_to_clean++;
        process.value()->live_threads--;
    } else {
        state = ThreadState::kWaitingForJoin;
    }
    SchedulingModule::Get().GetScheduler().ExitThreadUnguarded(state);
}

std::expected<void *, Error> TaskMgr::JoinThread(const Tid tid)
{
    const auto tcb = hardware::GetCoreLocalTcb();
    ASSERT_NOT_NULL(tcb);
    ASSERT_EQ(tcb->state, ThreadState::kRunning);

    if (tcb->tid == tid) {
        return std::unexpected(Error::SelfJoin);
    }

    LocalCoreLock lock{};
    auto thread = SchedulingModule::Get().GetThreads().GetThread(tid);
    RET_UNEXPECTED_IF_ERR(thread);

    while (thread.value()->state != ThreadState::kWaitingForJoin ||
           thread.value()->state != ThreadState::kTerminated) {
        static constexpr u64 kWaitTime = 50'000'000;  // 50 ms

        SchedulingModule::Get().GetScheduler().NanoSleepUntil(
            TimingModule::Get().GetSystemTime().ReadLifeTimeNs() + kWaitTime
        );
    }

    if (thread.value()->state == ThreadState::kWaitingForJoin) {
        thread.value()->state = ThreadState::kTerminated;
        threads_to_clean_.Push(thread.value()->tid.id);

        const auto process =
            SchedulingModule::Get().GetProcesses().GetProcess(thread.value()->owner);
        ASSERT_TRUE(static_cast<bool>(process));

        process.value()->threads_to_clean++;
        process.value()->live_threads--;

        return {thread.value()->retval};
    }

    return std::unexpected(Error::AlreadyJoined);
}

std::expected<Pid, Error> TaskMgr::Exec(const char *path)
{
    ASSERT_NOT_NULL(path);

    ProcessFlags flags{};
    flags.KernelSpaceOnly = false;
    flags.PreserveFloats  = true;

    LocalCoreLock lock{};
    const auto result = ExecuteElf64(path, flags);
    if (!result) {
        return std::unexpected(result.error());
    }

    const auto pid = std::get<Pid>(result.value());
    return pid;
}

std::expected<int, Error> TaskMgr::JoinProcess(const Pid pid)
{
    if (hardware::GetRunningPid() == pid) {
        return std::unexpected(Error::SelfJoin);
    }

    LocalCoreLock lock{};
    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    RET_UNEXPECTED_IF_ERR(process);

    while (process.value()->state != ProcessState::kWaitingForJoin ||
           process.value()->state != ProcessState::kTerminated) {
    }
}

void TaskMgr::ThreadRipperWork()
{
    while (threads_to_clean_.Size() != 0) {
        LocalCoreLock lock{};
        ThreadRipperClean_(threads_to_clean_.Pop());
    }
}

void TaskMgr::ProcessRipperWork()
{
    while (processes_to_clean_.Size() != 0) {
        LocalCoreLock lock{};

        ProcessRipperClean_(processes_to_clean_.Pop());
    }
}

void TaskMgr::ThreadRipperClean_(const u32 id)
{
    const auto thread = SchedulingModule::Get().GetThreads().GetThread(id);
    ASSERT_NOT_NULL(thread);

    DEBUG_INFO_SCHEDULING("ThreadRipper cleaning: %llu", thread.value()->tid);

    Mem::KFree(thread.value()->kernel_stack);

    if (thread.value()->user_stack != nullptr) {
        // TODO: remove
    }

    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(thread.value()->owner);
    process.value()->threads_to_clean--;

    const auto result = SchedulingModule::Get().GetThreads().Free(thread.value()->tid);
    ASSERT_TRUE(static_cast<bool>(result));
}

void TaskMgr::ProcessRipperClean_(const u32 id)
{
    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(id);
    ASSERT_TRUE(static_cast<bool>(process));

    DEBUG_INFO_SCHEDULING("ProcessRipper cleaning: %llu", process.value()->pid);

    ASSERT_ZERO(process.value()->live_threads);

    while (process.value()->threads_to_clean != 0) {
        SchedulingModule::Get().GetScheduler().Yield();
    }

    const auto result =
        MemoryModule::Get().GetVmm().DestroyUserAddrSpace(process.value()->address_space);
    ASSERT_TRUE(static_cast<bool>(result));

    const auto proc_result = SchedulingModule::Get().GetProcesses().Free(process.value()->pid);
    ASSERT_TRUE(static_cast<bool>(proc_result));
}
}  // namespace Sched
;
