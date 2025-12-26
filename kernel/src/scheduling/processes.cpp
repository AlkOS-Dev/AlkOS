#include "processes.hpp"
#include "error.hpp"

// ------------------------------
// statics
// ------------------------------

// ------------------------------
// Implementations
// ------------------------------

std::expected<Sched::Process *, Sched::Error> Sched::Processes::PrepareProcess()
{
    const size_t idx = processes_.Allocate();

    if (idx == std::numeric_limits<size_t>::max()) {
        return std::unexpected(Error::ExceededMaxAllowedInstances);
    }

    Process *process = processes_.Get(idx);

    ASSERT_LE(idx, std::numeric_limits<u16>::max());
    process->pid = AssignNewPid(static_cast<u16>(idx));

    return process;
}
