#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_CYCLIC_BUFFER_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_CYCLIC_BUFFER_HPP_

#include <algorithm.hpp>
#include <array.hpp>
#include <atomic.hpp>
#include <defines.hpp>
#include <mutex.hpp>
#include <span.hpp>
#include <types.hpp>

namespace data_structures
{

/**
 * @brief Dummy lock for single-threaded usage.
 * Compiler optimizes this away completely.
 */
struct NoLock {
    FORCE_INLINE_F void lock() {}
    FORCE_INLINE_F void unlock() {}
};

/**
 * @brief A generic Ring Buffer implementation using monotonic indices.
 *
 * Thread Safety:
 * - Status methods (Count, IsFull, IsEmpty) are wait-free and thread-safe via atomics.
 * - IO methods (Read, Write) use the provided LockT policy.
 *
 * @tparam T Data type.
 * @tparam kSize Buffer size. MUST be a power of 2.
 * @tparam LockT The locking primitive to use (e.g., hal::Spinlock). Use NoLock for single thread.
 */
template <typename T, size_t kSize, typename LockT = NoLock>
class CyclicBuffer
{
    static_assert((kSize != 0) && ((kSize & (kSize - 1)) == 0), "Size must be power of 2");

    public:
    using ValueType = T;
    using SizeType  = size_t;

    CyclicBuffer() = default;

    /**
     * @brief Writes data into the buffer.
     * Thread-safe regarding Multiple Producers/Consumers if LockT is valid.
     * @return Number of elements actually written.
     */
    SizeType Write(std::span<const T> data)
    {
        static_assert(
            std::is_copy_assignable_v<T>, "T must be copy assignable to use Write(const T)"
        );

        std::lock_guard guard(lock_);

        // Use relaxed load because we are inside the lock.
        // The lock acquisition provides the necessary Acquire semantics.
        size_t write_idx      = write_index_.load(std::memory_order_relaxed);
        const size_t read_idx = read_index_.load(std::memory_order_relaxed);

        const SizeType current_count = write_idx - read_idx;
        const SizeType free_space    = kSize - current_count;

        if (free_space == 0 || data.empty()) {
            return 0;
        }

        const SizeType to_write = std::min(data.size(), free_space);

        for (SizeType i = 0; i < to_write; ++i) {
            buffer_[write_idx & kMask] = data[i];
            write_idx++;
        }

        // Store relaxed. The lock release at the end of the scope provides
        // the necessary Release semantics/memory barrier.
        write_index_.store(write_idx, std::memory_order_relaxed);

        return to_write;
    }

    /**
     * @brief Writes data into the buffer (Move Semantics).
     * WARNING: The source data will be moved-from.
     * Thread-safe regarding Multiple Producers/Consumers if LockT is valid.
     * @return Number of elements actually written.
     */
    SizeType Write(std::span<T> data)
    {
        static_assert(std::is_move_assignable_v<T>, "T must be move assignable to use Write(T)");

        std::lock_guard guard(lock_);

        // Use relaxed load because we are inside the lock.
        // The lock acquisition provides the necessary Acquire semantics.
        size_t write_idx      = write_index_.load(std::memory_order_relaxed);
        const size_t read_idx = read_index_.load(std::memory_order_relaxed);

        const SizeType current_count = write_idx - read_idx;
        const SizeType free_space    = kSize - current_count;

        if (free_space == 0 || data.empty()) {
            return 0;
        }

        const SizeType to_write = std::min(data.size(), free_space);

        for (SizeType i = 0; i < to_write; ++i) {
            buffer_[write_idx & kMask] = std::move(data[i]);
            write_idx++;
        }

        // Store relaxed. The lock release at the end of the scope provides
        // the necessary Release semantics/memory barrier.
        write_index_.store(write_idx, std::memory_order_relaxed);

        return to_write;
    }

    /**
     * @brief Reads data from the buffer.
     * Thread-safe regarding Multiple Producers/Consumers if LockT is valid.
     * @return Number of elements actually read.
     */
    SizeType Read(std::span<T> out_buffer)
    {
        static_assert(std::is_move_assignable_v<T>, "T must be move assignable to use Read(T)");

        std::lock_guard guard(lock_);

        const size_t write_idx = write_index_.load(std::memory_order_relaxed);
        size_t read_idx        = read_index_.load(std::memory_order_relaxed);

        const SizeType current_count = write_idx - read_idx;

        if (current_count == 0 || out_buffer.empty()) {
            return 0;
        }

        const SizeType to_read = std::min(out_buffer.size(), current_count);

        for (SizeType i = 0; i < to_read; ++i) {
            out_buffer[i] = std::move(buffer_[read_idx & kMask]);
            read_idx++;
        }

        read_index_.store(read_idx, std::memory_order_relaxed);

        return to_read;
    }

    /**
     * @brief Returns an estimate of the number of elements in the buffer.
     * Wait-Free. No Lock acquired.
     */
    NODISCARD SizeType Count() const
    {
        // Safe to read without the lock because indices are atomic.
        // No Data Race (Undefined Behavior).
        // No Cache Thrashing on the lock variable.
        auto w = write_index_.load(std::memory_order_relaxed);
        auto r = read_index_.load(std::memory_order_relaxed);
        return w - r;
    }

    NODISCARD bool IsFull() const { return Count() == kSize; }

    NODISCARD bool IsEmpty() const { return Count() == 0; }

    NODISCARD SizeType Capacity() const { return kSize; }

    private:
    static constexpr size_t kMask = kSize - 1;

    // Indices are atomic to allow lock-free status checks (Count/IsFull) without UB.
    // They are updated inside the lock during Read/Write operations.
    std::atomic<size_t> write_index_{0};
    std::atomic<size_t> read_index_{0};

    std::array<T, kSize> buffer_{};
    LockT lock_{};
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_CYCLIC_BUFFER_HPP_
