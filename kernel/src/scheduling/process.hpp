#ifndef KERNEL_SRC_SCHEDULING_PROCESS_HPP_
#define KERNEL_SRC_SCHEDULING_PROCESS_HPP_

#include <defines.hpp>
#include <types.hpp>

namespace Sched
{
struct PACK Pid {
    u16 id;
    u64 count : 48;
};

struct Process {
    Pid pid;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_PROCESS_HPP_
