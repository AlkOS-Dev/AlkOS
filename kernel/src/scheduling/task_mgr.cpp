#include "task_mgr.hpp"
#include "macros.hpp"
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
    Error err{};

    // 1. Prepare internal structure for the process
    auto process = SchedulingModule::Get().GetProcesses().PrepareProcess();

    // 2. Prepare internal structure for execution unit - thread:
    auto thread = SchedulingModule::Get().GetThreads().PrepareThread();

    thread.value()->owner = process.value()->pid;

    auto kernel_stack = Mem::KMallocAligned({kKernelStackSize, kStackAlignment});

    thread.value()->kernel_stack =

        return process.value();

CLEANUP:
}
}  // namespace Sched
