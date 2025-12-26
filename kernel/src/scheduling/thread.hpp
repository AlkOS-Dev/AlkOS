#ifndef KERNEL_SRC_SCHEDULING_THREAD_HPP_
#define KERNEL_SRC_SCHEDULING_THREAD_HPP_

#include <defines.hpp>
#include <types.hpp>

#include "process.hpp"

namespace Sched
{

struct PACK Tid {
    u16 id;
    u64 count : 48;
};

struct PACK Thread {
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
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_THREAD_HPP_
