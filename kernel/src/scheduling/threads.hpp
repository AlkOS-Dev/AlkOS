#ifndef KERNEL_SRC_SCHEDULING_THREADS_HPP_
#define KERNEL_SRC_SCHEDULING_THREADS_HPP_

#include <data_structures/hash_maps.hpp>
#include <defines.hpp>
#include <expected.hpp>
#include <hal/interrupt_params.hpp>
#include <hal/sync.hpp>

#include "constants.hpp"
#include "error.hpp"
#include "scheduling/thread.hpp"

namespace Sched
{
class Threads
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Threads()  = default;
    ~Threads() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    std::expected<Thread *, Error> PrepareThread();

    NODISCARD FORCE_INLINE_F std::expected<Thread *, Error> GetThread(const u32 id)
    {
        auto ptr = threads_.Get(id);

        if (ptr == nullptr) {
            return std::unexpected(Error::ThreadNotFound);
        }

        return ptr;
    }

    NODISCARD FORCE_INLINE_F std::expected<Thread *, Error> GetThread(const Tid tid)
    {
        return GetThread(tid.id);
    }

    FORCE_INLINE_F std::expected<void, Error> Free(const Tid tid)
    {
        const u16 id = tid.id;

        if (threads_.Get(id) == nullptr) {
            return std::unexpected(Error::ThreadNotFound);
        }

        threads_.Free(id);
        return {};
    }

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    NODISCARD FORCE_INLINE_F Tid AssignNewTid(const u16 id)
    {
        Tid tid{};
        tid.id    = id;
        tid.count = hal::AtomicIncrement(&thread_counter_) - 1;

        return tid;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::PooledHashMap<Thread, kMaxThreads> threads_{};
    hal::Atomic64 thread_counter_{};
};

void KThreadEntrypoint(void (*f)());
void OnKThreadExit();

void Elf64EntryPoint(Pid pid, const char *path);
void UserThreadEntryPoint(thread_func_t func, void *arg);

void UpdateTcbOnInterruptEntry(hal::ExceptionData *data);
void UpdateTcbOnInterruptExit(Thread *thread);

NODISCARD FAST_CALL Task PrepareKThreadTask(void (*f)())
{
    Task task{};
    task.func       = reinterpret_cast<void *>(KThreadEntrypoint);
    task.args_count = 1;
    task.args       = {reinterpret_cast<u64>(f)};

    return task;
}

NODISCARD FAST_CALL Task PrepareElf64LoaderTask(Pid pid, const char *path)
{
    Task task{};
    task.func       = reinterpret_cast<void *>(Elf64EntryPoint);
    task.args_count = 2;
    task.args       = {*reinterpret_cast<u64 *>(&pid), reinterpret_cast<u64>(path)};

    return task;
}

NODISCARD FAST_CALL Task PrepareUserThreadTask(thread_func_t func, void *arg)
{
    Task task{};
    task.func       = reinterpret_cast<void *>(UserThreadEntryPoint);
    task.args_count = 2;
    task.args       = {reinterpret_cast<u64>(func), reinterpret_cast<u64>(arg)};

    return task;
}

}  // namespace Sched

extern "C" void cdecl_UpdateTcbOnSyscallEntry();
extern "C" void cdecl_UpdateTcbOnSyscallExit();

#endif  // KERNEL_SRC_SCHEDULING_THREADS_HPP_
