#ifndef KERNEL_SRC_SCHEDULING_THREAD_HPP_
#define KERNEL_SRC_SCHEDULING_THREAD_HPP_

#include <types.h>
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
    u32 padding : 31;
};
static_assert(sizeof(ThreadFlags) == 4);

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
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_THREAD_HPP_
