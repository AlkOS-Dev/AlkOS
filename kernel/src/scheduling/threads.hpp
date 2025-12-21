#ifndef KERNEL_SRC_SCHEDULING_THREADS_HPP_
#define KERNEL_SRC_SCHEDULING_THREADS_HPP_

#include <data_structures/hash_maps.hpp>
#include <defines.hpp>
#include <expected.hpp>
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

    NODISCARD FORCE_INLINE_F std::expected<Thread *, Error> GetThread(const Tid tid)
    {
        const u16 id = tid.id;
        auto ptr     = threads_.Get(id);

        if (ptr == nullptr) {
            return std::unexpected(Error::ThreadNotFound);
        }

        return ptr;
    }

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    NODISCARD FORCE_INLINE_F Tid AssignNewTid(const u16 id)
    {
        Tid tid{};
        tid.id    = id;
        tid.count = hal::AtomicIncrement(&thread_counter_);

        return tid;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    data_structures::PooledHashMap<Thread, kMaxThreads> threads_{};
    hal::Atomic64 thread_counter_{};
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_THREADS_HPP_
