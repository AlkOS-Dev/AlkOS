#ifndef KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_
#define KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_

#include <assert.h>
#include <data_structures/intrusive_linked_list.hpp>
#include <defines.hpp>

#include "scheduling/policy.hpp"
#include "scheduling/thread.hpp"

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

    NODISCARD FORCE_INLINE_F Thread *PickNextTask() { return threads_.PopFront(); }

    FORCE_INLINE_F void AddTask(Thread *thread)
    {
        ASSERT_EQ(thread->state, ThreadState::kReady);
        threads_.PushBack(thread);
    }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::IntrusiveList<Thread> threads_{};
};

}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_POLICIES_ROUND_ROBIN_POLICY_HPP_
