#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_PRIORITY_QUEUES_BITMAP_PQ_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_PRIORITY_QUEUES_BITMAP_PQ_HPP_

#include <array.hpp>
#include <concepts.hpp>

#include "data_structures/bit_array.hpp"
#include "data_structures/intrusive_linked_list.hpp"

namespace data_structures
{
template <class T, size_t kSize, int kIntrusiveLevel>
    requires std::derived_from<T, IntrusiveDoubleListNode<T, kIntrusiveLevel>>
class BitmapPriorityQueue
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    BitmapPriorityQueue()  = default;
    ~BitmapPriorityQueue() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F T *FindMin() const
    {
        const size_t idx = bits_.template FindFirst<true>();
        if (idx == std::numeric_limits<size_t>::max()) {
            return nullptr;
        }

        ASSERT_NOT_NULL(queues_[idx].Front());
        return queues_[idx].Front();
    }

    NODISCARD FORCE_INLINE_F T *FindMax() const
    {
        const size_t idx = bits_.template FindLast<true>();
        if (idx == std::numeric_limits<size_t>::max()) {
            return nullptr;
        }

        ASSERT_NOT_NULL(queues_[idx].Front());
        return queues_[idx].Front();
    }

    NODISCARD FORCE_INLINE_F T *DeleteMin()
    {
        const size_t idx = bits_.template FindFirst<true>();
        if (idx == std::numeric_limits<size_t>::max()) {
            return nullptr;
        }

        T *min = queues_[idx].PopFront();

        if (queues_[idx].IsEmpty()) {
            bits_.SetFalse(idx);
        }

        return min;
    }

    NODISCARD FORCE_INLINE_F T *DeleteMax()
    {
        const size_t idx = bits_.template FindLast<true>();
        if (idx == std::numeric_limits<size_t>::max()) {
            return nullptr;
        }

        T *min = queues_[idx].PopFront();

        if (queues_[idx].IsEmpty()) {
            bits_.SetFalse(idx);
        }

        return min;
    }

    FORCE_INLINE_F void Insert(T *item, const size_t priority)
    {
        ASSERT_LT(priority, kSize);
        queues_[priority].PushBack(item);
    }

    FORCE_INLINE_F void Remove(T *item, const size_t priority)
    {
        ASSERT_LT(priority, kSize);
        queues_[priority].Remove(item);
    }

    NODISCARD FORCE_INLINE_F bool IsEmpty() const { return bits_.IsAllFalse(); }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    // ------------------------------
    // Class fields
    // ------------------------------

    BitArray<kSize> bits_{};
    IntrusiveDoubleList<T, kIntrusiveLevel> queues_[kSize]{};
};

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_PRIORITY_QUEUES_BITMAP_PQ_HPP_
