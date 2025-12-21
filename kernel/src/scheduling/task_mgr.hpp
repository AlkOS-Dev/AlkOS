#ifndef KERNEL_SRC_SCHEDULING_TASK_MGR_HPP_
#define KERNEL_SRC_SCHEDULING_TASK_MGR_HPP_

#include <expected.hpp>
#include "error.hpp"
#include "process.hpp"

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

    NODISCARD std::expected<Process *, Error> SpawnProcess();

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
