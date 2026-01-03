#ifndef KERNEL_SRC_SCHEDULING_THREAD_HPP_
#define KERNEL_SRC_SCHEDULING_THREAD_HPP_

#include <types.h>
#include <array.hpp>
#include <data_structures/intrusive_linked_list.hpp>
#include <data_structures/maps/intrusive_rb_tree.hpp>
#include <defines.hpp>

#include "hal/tasks.hpp"
#include "policy.hpp"
#include "process.hpp"

namespace Sched
{
// ------------------------------
// Task
// ------------------------------

struct Task {
    static constexpr size_t kMaxArgs = 6;

    void *func;
    std::array<u64, kMaxArgs> args;
    size_t args_count;
};

// ------------------------------
// Thread
// ------------------------------

struct PACK Tid {
    u16 id;
    u64 count : 48;
};

struct PACK ThreadFlags {
    SchedulingPolicy policy : 8;
    u8 priority : 8;
    bool PreserveFloats : 1;
    u64 padding : 47;
};
static_assert(sizeof(ThreadFlags) == 8);

enum class ThreadState : u64 {
    kReady = 0,
    kRunning,
    kSleeping,
    kTerminated,
    kLast,
};
static_assert(sizeof(ThreadState) == sizeof(u64));

struct Thread : data_structures::IntrusiveRbNode<Thread, u64>,
                data_structures::IntrusiveListNode<Thread> {
    /* Management */
    Tid tid;
    Pid owner;
    ThreadFlags flags;
    ThreadState state;

    /* Thread resources */
    void *kernel_stack;
    void *kernel_stack_bottom;
    void *user_stack;
    void *user_stack_bottom;

    /* Statistics */
    u64 kernel_time_ns;
    u64 user_time_ns;
    u64 timestamp;
    u64 num_interrupts;
    u64 num_syscalls;
    u64 num_context_switches;
    u64 padding0;

    /* Arch */
    hal::Thread arch_data;
};
static_assert(
    sizeof(Thread) ==
    (128 + sizeof(hal::Thread) + sizeof(data_structures::IntrusiveRbNode<Thread, u64>))
);
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_THREAD_HPP_
