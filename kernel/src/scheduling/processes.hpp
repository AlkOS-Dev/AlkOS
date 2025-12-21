#ifndef KERNEL_SRC_SCHEDULING_PROCESSES_HPP_
#define KERNEL_SRC_SCHEDULING_PROCESSES_HPP_

#include <data_structures/hash_maps.hpp>
#include <expected.hpp>

#include "constants.hpp"
#include "hal/sync.hpp"
#include "scheduling/error.hpp"
#include "scheduling/process.hpp"

namespace Sched
{
class Processes
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Processes()  = default;
    ~Processes() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    std::expected<Process *, Error> PrepareProcess();

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    NODISCARD FORCE_INLINE_F Pid AssignNewPid(const u16 id)
    {
        Pid pid{};
        pid.id    = id;
        pid.count = hal::AtomicIncrement(&process_counter_);

        return pid;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::PooledHashMap<Process, kMaxProcesses> processes_{};
    hal::Atomic64 process_counter_{};
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_PROCESSES_HPP_
