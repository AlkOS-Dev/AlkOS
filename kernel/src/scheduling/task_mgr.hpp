#ifndef KERNEL_SRC_SCHEDULING_TASK_MGR_HPP_
#define KERNEL_SRC_SCHEDULING_TASK_MGR_HPP_

#include <expected.hpp>
#include "error.hpp"
#include "process.hpp"
#include "thread.hpp"

namespace Sched
{
class TaskMgr
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    TaskMgr()  = default;
    ~TaskMgr() = default;

    // ------------------------------
    // Initialization
    // ------------------------------

    void InitializeMultitasking();

    // ------------------------------
    // Spawning
    // ------------------------------

    NODISCARD std::expected<Pid, Error> SpawnEmptyProcess(const char *name, ProcessFlags flags);

    NODISCARD std::expected<Thread *, Error> SpawnThread(Pid pid, const Task &task);

    NODISCARD std::expected<Thread *, Error> SpawnThread(
        Pid pid, ThreadFlags flags, const Task &task
    );

    NODISCARD std::expected<Thread *, Error> SpawnThread(
        Process *process, ThreadFlags flags, const Task &task
    );

    NODISCARD std::expected<std::tuple<Pid, Tid>, Error> SpawnKernelProcess(
        const char *name, ProcessFlags flags, const Task &task
    );

    NODISCARD std::expected<Pid, Error> CloneProcess(Pid pid);

    // ------------------------------
    // Execute
    // ------------------------------

    NODISCARD std::expected<Tid, Error> ExecuteElf64(Pid pid, const char *path);

    NODISCARD std::expected<std::tuple<Pid, Tid>, Error> ExecuteElf64(
        const char *path, ProcessFlags flags
    );

    // ------------------------------
    // Syscalls
    // ------------------------------

    NODISCARD std::expected<void, Error> CommitMurder(Tid tid);

    NODISCARD std::expected<void, Error> CommitMurder(Pid pid);

    void CommitSuicide();

    NODISCARD std::expected<void, Error> ExitProcess();

    NODISCARD std::expected<Tid, Error> CreateThread(ThreadFlags flags, const Task &task);

    NODISCARD std::expected<Tid, Error> CreateUserThread(
        ThreadFlags flags, thread_func_t func, void *arg
    );

    NODISCARD std::expected<void, Error> DetachThread(Tid tid);

    void ThreadExit(void *retval);

    NODISCARD std::expected<void *, Error> JoinThread(Tid tid);

    NODISCARD std::expected<Pid, Error> Exec(const char *path);

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    // ------------------------------
    // Class fields
    // ------------------------------

    AtomicArraySingleTypeStaticStack<u32, kMaxThreads> threads_to_clean_{};
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_TASK_MGR_HPP_
