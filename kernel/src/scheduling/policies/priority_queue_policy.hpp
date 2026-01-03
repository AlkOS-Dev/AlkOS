#ifndef KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_

#include "array.hpp"
#include "scheduling/policy.hpp"

namespace Sched
{

class PriorityQueuePolicy : public PolicyImpl
{
    struct queue {
        Thread *head;
        Thread *tail;
    };

    public:
    static constexpr u8 kMaxPriority = 64;

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

    std::array<queue, kMaxPriority> queue_;
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_PRIORITY_QUEUE_POLICY_HPP_
