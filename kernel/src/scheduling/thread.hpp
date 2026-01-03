#ifndef KERNEL_SRC_SCHEDULING_THREAD_HPP_
#define KERNEL_SRC_SCHEDULING_THREAD_HPP_

#include <types.h>
#include <array.hpp>
#include <defines.hpp>

#include "hal/tasks.hpp"
#include "process.hpp"

namespace Sched
{

struct PACK Tid {
    u16 id;
    u64 count : 48;
};

struct PACK ThreadFlags {
    bool PreserveFloats : 1;
    u64 padding : 63;
};
static_assert(sizeof(ThreadFlags) == 8);

struct Task {
    static constexpr size_t kMaxArgs = 6;

    void *func;
    std::array<u64, kMaxArgs> args;
    size_t args_count;
};

struct Thread : hal::Thread {
    /* Management */
    Tid tid;
    Pid owner;
    ThreadFlags flags;

    /* Scheduler data */
    Thread *next;

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
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_THREAD_HPP_
