#ifndef KERNEL_SRC_SCHEDULING_THREAD_HPP_
#define KERNEL_SRC_SCHEDULING_THREAD_HPP_

#include <array.hpp>
#include <defines.hpp>
#include <types.hpp>

#include "hal/tasks.hpp"
#include "process.hpp"

namespace Sched
{

struct PACK Tid {
    u16 id;
    u64 count : 48;
};

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

    /* Scheduler data */
    Thread *next;

    /* Thread resources */
    void *kernel_stack;
    void *kernel_stack_bottom;
    void *user_stack;
    void *user_stack_bottom;

    /* Timing */
    u64 kernel_time_ns;
    u64 user_time_ns;
    u64 timestamp;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_THREAD_HPP_
