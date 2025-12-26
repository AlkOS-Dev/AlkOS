#ifndef KERNEL_SRC_SCHEDULING_PROCESS_HPP_
#define KERNEL_SRC_SCHEDULING_PROCESS_HPP_

#include <defines.hpp>
#include <types.hpp>

#include "mem/virt/addr_space.hpp"

namespace Sched
{
struct PACK Pid {
    u16 id;
    u64 count : 48;
};

struct PACK ProcessFlags {
    bool KernelSpaceOnly : 1;
};
static_assert(sizeof(ProcessFlags) == 1);

struct Process {
    /* Management */
    Pid pid;
    ProcessFlags flags;

    /* Process resources */
    Mem::VirtualPtr<Mem::AddressSpace> address_space;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_PROCESS_HPP_
