#include "task_mgr.hpp"
#include "modules/scheduling.hpp"

// ------------------------------
// statics
// ------------------------------

// ------------------------------
// Implementations
// ------------------------------

namespace Sched
{
void TaskMgr::InitializeMultitasking() {}

std::expected<Process *, Error> TaskMgr::SpawnProcess()
{
    // 1. Prepare internal structure for the process
    auto process = SchedulingModule::Get().GetProcesses().PrepareProcess();

    if (!process) {
        return std::unexpected(process.error());
    }

    // 2. Prepare internal structure for execution unit - thread:
    auto thread = SchedulingModule::Get().GetThreads().PrepareThread();

    if (!thread) {
        return std::unexpected(thread.error());
    }

    thread.value()->owner = process.value()->pid;

    return process.value();
}
}  // namespace Sched
