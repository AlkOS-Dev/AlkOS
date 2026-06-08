#include "threads.hpp"

#include <hal/debug_terminal.hpp>
#include <hal/scheduling.hpp>
#include <mem/virt/vmm.hpp>
#include <modules/memory.hpp>

#include "fs/vfs/path.hpp"
#include "hardware/core_local.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/scheduling.hpp"
#include "modules/timing.hpp"
#include "scheduling/local_lock.hpp"
#include "sys/loader.hpp"
#include "template/scope_guard.hpp"

namespace Sched
{
std::expected<Thread *, Error> Threads::PrepareThread()
{
    const size_t idx = threads_.Allocate();
    template_lib::ScopeGuard thread_guard([&]() {
        threads_.Free(idx);
    });

    if (idx == std::numeric_limits<size_t>::max()) {
        return std::unexpected(Error::ExceededMaxAllowedInstances);
    }

    Thread *thread = threads_.Get(idx);
    ASSERT_LE(idx, std::numeric_limits<u16>::max());
    thread->tid = AssignNewTid(static_cast<u16>(idx));

    // Allocate wait queue
    const auto wait_queue = Mem::KNew<WaitQueue<Thread, kWaitQueueIntrusiveLevel>>();
    if (!wait_queue) {
        return std::unexpected(Error::OutOfMemory);
    }
    thread->wait_queue = wait_queue.value();

    thread_guard.Dismiss();
    return thread;
}

std::expected<void, Error> Threads::Free(const Tid tid)
{
    const u16 id = tid.id;

    const auto thread = threads_.Get(id);
    if (thread == nullptr) {
        return std::unexpected(Error::ThreadNotFound);
    }

    ASSERT_NOT_NULL(thread->wait_queue);
    ASSERT_TRUE(thread->wait_queue->IsEmpty());
    Mem::KDelete(thread->wait_queue);
    threads_.Free(id);

    TRACE_INFO_SCHEDULING("Fully freed thread with TID: %llu", tid);
    return {};
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
    ASSERT_NOT_NULL(path);
    LocalCoreLock core_lock{};

    const auto process = SchedulingModule::Get().GetProcesses().GetProcess(pid);
    ASSERT_TRUE(static_cast<bool>(process));  // TODO: CAN IT BE MISSING HERE????

    auto as = process.value()->address_space;
    ASSERT_NOT_NULL(as);

    const auto entry_res = System::ElfLoader::Load(vfs::Path(path), *as);
    Mem::KFree(reinterpret_cast<void *>(reinterpret_cast<u64>(path)));

    if (!entry_res) {
        DEBUG_WARN_SCHEDULING(
            "Failed to execute ELF64 for process %llu. Failed on ELF loading: %s.", pid,
            to_string(entry_res.error())
        );
        SchedulingModule::Get().GetTaskMgr().CommitSuicide();
    }

    const auto entry = static_cast<void *>(entry_res.value());

    auto &vmm           = MemoryModule::Get().GetVmm();
    const auto heap_res = vmm.AllocUserHeap(as, kHeapSize);
    if (!heap_res) {
        DEBUG_WARN_SCHEDULING(
            "Failed to execute ELF64 for process %llu. Failed on heap allocation.", pid
        );
        SchedulingModule::Get().GetTaskMgr().CommitSuicide();
    }
    process.value()->heap_start = *heap_res;
    hal::JumpToUserSpace(entry, nullptr);
}

void UserThreadEntryPoint(const thread_func_t func, void *arg)
{
    hal::JumpToUserSpace(reinterpret_cast<void *>(func), arg);
}

void UpdateTcbOnInterruptEntry(hal::ExceptionData *data)
{
    ASSERT_NOT_NULL(data);
    const auto thread = hardware::GetCoreLocalTcb();

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
    thread->num_interrupts++;
}

void UpdateTcbOnInterruptExit(Thread *thread)
{
    const auto current_thread = hardware::GetCoreLocalTcb();

    if (!current_thread) {
        ASSERT_NULL(thread);
        return;
    }

    const u64 t = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    current_thread->kernel_time_ns += t - current_thread->timestamp;
    current_thread->timestamp = t;
    current_thread->num_context_switches++;

    if (thread) {
        /* Context Switch occurs */
        thread->timestamp                    = t;
        thread->timestamp_execution_start_ns = t;

        ASSERT_EQ(current_thread->state, ThreadState::kReady);
        ASSERT_EQ(thread->state, ThreadState::kRunning);
    } else {
        /* No context switch occurred */
        ASSERT_EQ(current_thread->state, ThreadState::kRunning);
    }
}

}  // namespace Sched

void cdecl_UpdateTcbOnSyscallEntry()
{
    const auto thread = hardware::GetCoreLocalTcb();
    const u64 t       = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    thread->user_time_ns += t - thread->timestamp;
    thread->timestamp = t;
    thread->num_syscalls++;
}

void cdecl_UpdateTcbOnSyscallExit()
{
    const auto thread = hardware::GetCoreLocalTcb();
    const u64 t       = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    thread->kernel_time_ns += t - thread->timestamp;
    thread->timestamp = t;
}
