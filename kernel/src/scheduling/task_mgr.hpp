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

    NODISCARD std::expected<std::tuple<Pid, Tid>, Error> SpawnProcess(
        void (*f)(), ProcessFlags flags
    );

    NODISCARD std::expected<Thread *, Error> SpawnThread(Pid pid, void (*f)());

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
