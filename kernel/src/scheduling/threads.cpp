#include "threads.hpp"

#include <hal/scheduling.hpp>

#include "hardware/core_local.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"
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

void UpdateTcbOnInterruptEntry(hal::ExceptionData *data)
{
    ASSERT_NOT_NULL(data);
    const auto thread = hardware::GetCurrentTCB();

    if (!thread) {
        return;
    }

    const u64 t = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    if (hal::IsInterruptFromUserSpace(*data)) {
        thread->user_time_ns += t - thread->timestamp;
    } else {
        thread->kernel_time_ns += t - thread->timestamp;
    }

    thread->timestamp = t;
}

void UpdateTcbOnInterruptExit(Thread *thread)
{
    const auto current_thread = hardware::GetCurrentTCB();

    if (!current_thread) {
        return;
    }

    const u64 t = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    current_thread->kernel_time_ns += t - thread->timestamp;
    thread->timestamp = t;
}

}  // namespace Sched

void cdecl_UpdateTcbOnSyscallEntry()
{
    const auto thread = hardware::GetCurrentTCB();
    const u64 t       = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    thread->user_time_ns += t - thread->timestamp;
    thread->timestamp = t;
}

void cdecl_UpdateTcbOnSyscallExit()
{
    const auto thread = hardware::GetCurrentTCB();
    const u64 t       = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    thread->kernel_time_ns += t - thread->timestamp;
    thread->timestamp = t;
}
