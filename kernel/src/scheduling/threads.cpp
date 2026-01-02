#include "threads.hpp"

#include <hal/debug_terminal.hpp>
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
    thread->InitMem();

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
