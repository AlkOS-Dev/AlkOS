#ifndef KERNEL_SRC_SCHEDULING_PROCESSES_HPP_
#define KERNEL_SRC_SCHEDULING_PROCESSES_HPP_

#include <data_structures/hash_maps.hpp>
#include "constants.hpp"
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

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    data_structures::PooledHashMap<Process, kMaxProcesses> processes_{};
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_PROCESSES_HPP_
