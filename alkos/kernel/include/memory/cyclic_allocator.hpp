#ifndef ALKOS_KERNEL_INCLUDE_MEMORY_CYCLIC_ALLOCATOR_HPP_
#define ALKOS_KERNEL_INCLUDE_MEMORY_CYCLIC_ALLOCATOR_HPP_

#include <stddef.h>
#include <extensions/bit_array.hpp>
#include <extensions/compare.hpp>
#include <extensions/new.hpp>
#include <todo.hpp>
#include "sync/kernel/spinlock.hpp"

TODO_WHEN_VMEM_WORKS
/* TODO: STATIC MEMORY ALLOCATOR!!! */

template <class T, size_t kNumObjects>
class CyclicAllocator final
{
    using StorageT = std::aligned_storage_t<sizeof(T), alignof(T)>;

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    CyclicAllocator() = default;

    ~CyclicAllocator() { ASSERT_EQ(kNumObjects, free_slots_, "Memory leak in CyclicAllocator"); }

    // ------------------------------
    // Class methods
    // ------------------------------

    template <typename... Args>
    NODISCARD T *Allocate(Args &&...args)
    {
        R_ASSERT_NOT_ZERO(free_slots_, "Out of memory in CyclicAllocator");

        lock_.Lock();

        size_t cursor = cursor_;
        while (used_map_.Get(cursor)) {
            cursor = (cursor + 1) % kNumObjects;
        }

        used_map_.Set(cursor, true);
        cursor_ = (cursor + 1) % kNumObjects;
        ASSERT_NOT_ZERO(free_slots_, "Out of memory in CyclicAllocator");
        --free_slots_;

        lock_.Unlock();

        void *mem = &mem_[cursor];
        new (mem) T(std::forward<Args>(args)...);
        return static_cast<T *>(mem);
    }

    void Free(T *ptr)
    {
        const size_t idx =
            static_cast<size_t>(reinterpret_cast<byte *>(ptr) - reinterpret_cast<byte *>(mem_)) /
            sizeof(StorageT);
        ASSERT_LT(idx, kNumObjects, "Invalid pointer in CyclicAllocator.Free()");
        ASSERT_TRUE(used_map_.Get(idx));

        ptr->~T();

        lock_.Lock();
        used_map_.Set(idx, false);
        ++free_slots_;
        lock_.Unlock();
    }

    NODISCARD FORCE_INLINE_F size_t GetFreeSlots() const { return free_slots_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    BitArray<kNumObjects> used_map_{};
    StorageT mem_[kNumObjects]{};
    Spinlock lock_{};
    size_t free_slots_ = kNumObjects;
    size_t cursor_{};
};

#endif  // ALKOS_KERNEL_INCLUDE_MEMORY_CYCLIC_ALLOCATOR_HPP_
