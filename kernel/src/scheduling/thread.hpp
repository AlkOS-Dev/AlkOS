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

struct Thread {
    Tid tid;
    Pid owner;

    Thread *next;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_THREAD_HPP_
