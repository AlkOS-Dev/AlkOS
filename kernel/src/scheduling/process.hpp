#ifndef KERNEL_SRC_SCHEDULING_PROCESS_HPP_
#define KERNEL_SRC_SCHEDULING_PROCESS_HPP_

#include <types.h>
#include <defines.hpp>

#include "fs/costants.hpp"
#include "fs/vfs/path.hpp"
#include "hal/tasks.hpp"
#include "io/pipe.hpp"
#include "mem/types.hpp"

namespace Mem
{
class AddressSpace;
}

// Forward declarations
namespace Fs
{
class FdTable;
}

namespace Sched
{
struct Thread;

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

enum class ProcessState : u64 {
    kReady = 0,
    kWaitingForJoin,
    kTerminated,
    kLast,
};
static_assert(sizeof(ProcessState) == sizeof(u64));

struct Process : hal::Process {
    static constexpr size_t kMaxNameLength = vfs::kMaxComponentSize;

    /* Management */
    char name[kMaxNameLength];
    Pid pid;
    ProcessFlags flags;
    Thread *threads;
    u64 live_threads;
    u64 threads_to_clean;
    ProcessState state;
    int status;

    /* Process resources */
    Mem::VPtr<Mem::AddressSpace> address_space;

    /* File descriptor table */
    Mem::VPtr<Fs::FdTable> fd_table;

    /* Standard I/O pipes (owned by process) */
    IO::Pipe<Fs::kStdioBufferSize> stdin_pipe;
    IO::Pipe<Fs::kStdioBufferSize> stdout_pipe;
    IO::Pipe<Fs::kStdioBufferSize> stderr_pipe;
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_PROCESS_HPP_
