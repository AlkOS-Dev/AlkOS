#ifndef KERNEL_SRC_SCHEDULING_PROCESS_HPP_
#define KERNEL_SRC_SCHEDULING_PROCESS_HPP_

#include <defines.hpp>
#include <types.hpp>

#include "hal/tasks.hpp"
#include "mem/types.hpp"

namespace Mem
{
class AddressSpace;
}

namespace Sched
{
struct PACK Pid {
    u16 id;
    u64 count : 48;

    bool operator==(const Pid &other) const = default;
};

struct PACK ProcessFlags {
    bool KernelSpaceOnly : 1;
    bool PreserveFloats : 1;
};
static_assert(sizeof(ProcessFlags) == 1);

struct Process : hal::Process {
    static constexpr size_t kMaxNameLength = 128;

    /* Management */
    char name[kMaxNameLength];
    Pid pid;
    ProcessFlags flags;

    /* Process resources */
    Mem::VirtualPtr<Mem::AddressSpace> address_space;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_PROCESS_HPP_
