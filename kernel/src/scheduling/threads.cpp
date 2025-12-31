#include "threads.hpp"

#include <hal/scheduling.hpp>

#include "mem/virt/addr_space.hpp"
#include "modules/scheduling.hpp"
#include "sys/loader.hpp"
#include "vfs/path.hpp"

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

void OnKThreadExit()
{
    R_FAIL_ALWAYS("Not implemented. KThread should never return at this stage...");
}

void Elf64EntryPoint(const Pid pid, const char *path)
{
    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    ASSERT_TRUE(static_cast<bool>(process));  // TODO: CAN IT BE MISSING HERE????

    auto &as = process.value()->address_space;
    ASSERT_NOT_NULL(as);

    const auto entry_res = System::ElfLoader::Load(vfs::Path(path), *as);
    if (!entry_res) {
        DEBUG_WARN_SCHEDULING(
            "Failed to execute ELF64 for process %llu. Failed on ELF loading.", pid
        );
        SchedulingModule::Get().GetTaskMgr().CommitSuicide(pid);
    }

    const auto entry = reinterpret_cast<void (*)()>(entry_res.value());
    hal::JumpToUserSpace(entry);
}

}  // namespace Sched
