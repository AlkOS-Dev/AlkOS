#include "threads.hpp"

#include <hal/scheduling.hpp>

#include "mem/virt/addr_space.hpp"
#include "modules/scheduling.hpp"

namespace Sched
{
std::expected<Thread *, Error> Threads::PrepareThread()
{
    const size_t idx = threads_.Allocate();

    if (idx == std::numeric_limits<size_t>::max()) {
        return std::unexpected(Error::ExceededMaxAllowedInstances);
    }

    Thread *thread = threads_.Get(idx);

    ASSERT_LE(idx, std::numeric_limits<u16>::max());
    thread->tid = AssignNewTid(static_cast<u16>(idx));

    return thread;
}

void KThreadEntrypoint(void (*f)())
{
    f();
    OnKThreadExit();
}

void UserThreadEntrypoint(void (*f)())
{
    OnUserThreadEntry();
    hal::JumpToUserSpace(f);
}

void OnUserThreadEntry() {}

void OnKThreadExit()
{
    R_FAIL_ALWAYS("Not implemented. KThread should never return at this stage...");
}

}  // namespace Sched

void *cdecl_GetThreadsPageTable(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    const auto proc = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(proc), "Threads exists -> owner MUST exist");

    return proc.value()->address_space->PageTableRoot();
}
