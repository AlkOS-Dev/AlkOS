#ifndef KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_

#include <defines.hpp>

#include "scheduling/policy.hpp"

namespace Sched
{

class RoundRobinPolicy : public PolicyImpl
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    RoundRobinPolicy()  = default;
    ~RoundRobinPolicy() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD Thread *PickNextTask();

    void AddTask(Thread *thread);

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    Thread *head = nullptr;
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_
