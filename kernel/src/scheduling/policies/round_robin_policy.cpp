#include "round_robin_policy.hpp"
#include "scheduling/thread.hpp"

// ------------------------------
// Implementations
// ------------------------------

namespace Sched
{
Thread *RoundRobinPolicy::PickNextTask()
{
    if (head_ == nullptr) {
        return nullptr;
    }

    Thread *thread = head_;
    ASSERT_NOT_NULL(thread);
    ASSERT_EQ(thread->state, ThreadState::kReady);

    head_ = thread->next;
    return thread;
}

void RoundRobinPolicy::AddTask(Thread *thread)
{
    ASSERT_EQ(thread->state, ThreadState::kReady);

    if (head_ == nullptr) {
        head_        = thread;
        tail_        = thread;
        thread->next = nullptr;
        return;
    }

    tail_->next  = thread;
    thread->next = nullptr;
    tail_        = thread;
}
}  // namespace Sched
