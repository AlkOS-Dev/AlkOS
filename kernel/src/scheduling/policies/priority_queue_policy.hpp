#ifndef KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_

#include "scheduling/policy.hpp"

namespace Sched
{

class PriorityQueuePolicy : public PolicyImpl
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    PriorityQueuePolicy()  = default;
    ~PriorityQueuePolicy() = default;

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
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_
