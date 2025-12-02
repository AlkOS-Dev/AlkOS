#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_ATOMIC_CYCLIC_BUFFER_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_ATOMIC_CYCLIC_BUFFER_HPP_

#include <array.hpp>
#include <atomic.hpp>
#include <defines.hpp>
#include <hal/constants.hpp>
#include <span.hpp>

namespace data_structures
{

/**
 * @brief A lock-free, wait-free Ring Buffer for Single-Producer Single-Consumer (SPSC).
 *
 * @tparam T Type of data
 * @tparam kSize Buffer size. MUST be a power of 2 for bitmask wrapping optimization.
 */
template <typename T, size_t kSize>
class AtomicCyclicBuffer
{
    static_assert((kSize != 0) && ((kSize & (kSize - 1)) == 0), "Size must be power of 2");

    public:
    using ValueType = T;
    using SizeType  = size_t;

    AtomicCyclicBuffer() = default;

    /**
     * @brief Writes data into the buffer. Thread-safe for ONE producer.
     */
    SizeType Write(std::span<const T> data)
    {
        static_assert(
            std::is_copy_assignable_v<T>, "T must be copy assignable to use Write(const T)"
        );

        // memory_order_acquire guarantees we see the reads done by the consumer
        // before we overwrite that memory.
        const auto read_idx = read_index_.load(std::memory_order_acquire);
        auto write_idx      = write_index_.load(std::memory_order_relaxed);

        const auto size  = data.size();
        SizeType written = 0;

        // Since indices grow indefinitely, capacity - (write - read) is free space.
        const auto count      = write_idx - read_idx;
        const auto free_space = kSize - count;

        if (free_space == 0) {
            return 0;
        }

        const auto to_write = std::min(size, free_space);

        // We do this BEFORE updating the write_index_ to ensure the consumer
        // doesn't see the new index before the data is valid.
        for (size_t i = 0; i < to_write; ++i) {
            buffer_[write_idx & kMask] = data[i];
            write_idx++;
        }

        // memory_order_release guarantees that the data writes above are visible
        // to the consumer before they see the updated index.
        write_index_.store(write_idx, std::memory_order_release);

        return to_write;
    }

    /**
     * @brief Writes data into the buffer (Move Semantics). Thread-safe for ONE producer.
     * WARNING: The source data will be moved-from.
     */
    SizeType Write(std::span<T> data)
    {
        static_assert(std::is_move_assignable_v<T>, "T must be move assignable to use Write(T)");

        // memory_order_acquire guarantees we see the reads done by the consumer
        // before we overwrite that memory.
        const auto read_idx = read_index_.load(std::memory_order_acquire);
        auto write_idx      = write_index_.load(std::memory_order_relaxed);

        const auto size  = data.size();
        SizeType written = 0;

        // Since indices grow indefinitely, capacity - (write - read) is free space.
        const auto count      = write_idx - read_idx;
        const auto free_space = kSize - count;

        if (free_space == 0) {
            return 0;
        }

        const auto to_write = std::min(size, free_space);

        // We do this BEFORE updating the write_index_ to ensure the consumer
        // doesn't see the new index before the data is valid.
        for (size_t i = 0; i < to_write; ++i) {
            buffer_[write_idx & kMask] = std::move(data[i]);
            write_idx++;
        }

        // memory_order_release guarantees that the data writes above are visible
        // to the consumer before they see the updated index.
        write_index_.store(write_idx, std::memory_order_release);

        return to_write;
    }

    /**
     * @brief Reads data from the buffer. Thread-safe for ONE consumer.
     */
    SizeType Read(std::span<T> out_buffer)
    {
        static_assert(std::is_move_assignable_v<T>, "T must be move assignable to use Read(T)");

        // memory_order_acquire guarantees we see the data writes performed by the producer.
        const auto write_idx = write_index_.load(std::memory_order_acquire);
        auto read_idx        = read_index_.load(std::memory_order_relaxed);

        const auto count = write_idx - read_idx;

        if (count == 0) {
            return 0;
        }

        const auto to_read = std::min(out_buffer.size(), count);

        for (size_t i = 0; i < to_read; ++i) {
            out_buffer[i] = std::move(buffer_[read_idx & kMask]);
            read_idx++;
        }

        // memory_order_release guarantees the producer sees we are done with this memory.
        read_index_.store(read_idx, std::memory_order_release);

        return to_read;
    }

    NODISCARD SizeType Count() const
    {
        auto w = write_index_.load(std::memory_order_relaxed);
        auto r = read_index_.load(std::memory_order_relaxed);
        return w - r;
    }

    NODISCARD bool IsFull() const { return Count() == kSize; }

    NODISCARD bool IsEmpty() const { return Count() == 0; }

    NODISCARD SizeType Capacity() const { return kSize; }

    private:
    static constexpr size_t kMask = kSize - 1;

    // Use cache line alignment to prevent False Sharing between cores.
    // If write_index_ and read_index_ sit on the same cache line,
    // the cores will fight over ownership of that line (cache thrashing).

    alignas(hal::kCacheLineSizeBytes) std::atomic<size_t> write_index_{0};
    alignas(hal::kCacheLineSizeBytes) std::atomic<size_t> read_index_{0};

    std::array<T, kSize> buffer_{};
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_ATOMIC_CYCLIC_BUFFER_HPP_
