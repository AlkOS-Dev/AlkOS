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
    // Class interaction
    // ------------------------------

    void InitializeMultitasking();

    NODISCARD std::expected<Pid, Error> SpawnEmptyProcess(const char *name, ProcessFlags flags);

    NODISCARD std::expected<Thread *, Error> SpawnThread(Pid pid, void (*f)());

    NODISCARD std::expected<std::tuple<Pid, Tid>, Error> SpawnKernelProcess(
        const char *name, ProcessFlags flags, void (*f)()
    );

    NODISCARD std::expected<Pid, Error> CloneProcess(Pid pid);

    NODISCARD std::expected<Tid, Error> ExecuteElf64(Pid pid, const char *path);

    NODISCARD std::expected<std::tuple<Pid, Tid>, Error> ExecuteElf64(
        const char *path, ProcessFlags flags
    );

    NODISCARD std::expected<void, Error> KillProcess(Pid pid);

    NODISCARD std::expected<void, Error> ExitProcess(Pid pid);

    NODISCARD std::expected<void, Error> ExitThread(Tid tid);

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    // ------------------------------
    // Class fields
    // ------------------------------
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_TASK_MGR_HPP_
