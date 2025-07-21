#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_POOL_ALLOCATOR_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_POOL_ALLOCATOR_HPP_

#include <extensions/bit_array.hpp>
#include <extensions/type_traits.hpp>
#include <sync/kernel/spinlock.hpp>

template <size_t kChunkSize, size_t kChunks, size_t kAlignment = alignof(std::max_align_t)>
class PoolAllocator
{
    using StorageT = std::aligned_storage_t<kChunkSize, kAlignment>;

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    PoolAllocator() = default;
    ~PoolAllocator() { ASSERT_EQ(kChunks, free_slots_, "Memory leak in PoolAllocator"); }

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(size_t n)
    {
        if (free_slots_ == 0 || n > kChunkSize) {
            return nullptr;
        }

        lock_.Lock();

        size_t cursor = cursor_;
        while (used_map_.Get(cursor)) {
            cursor = (cursor + 1) % kChunks;
        }

        used_map_.Set(cursor, true);
        cursor_ = (cursor + 1) % kChunks;
        --free_slots_;

        lock_.Unlock();

        return {&mem_[cursor], kChunkSize};
    }

    constexpr void Deallocate(AllocatorBlock block)
    {
        const size_t idx =
            static_cast<size_t>(
                reinterpret_cast<byte *>(block.ptr) - reinterpret_cast<const byte *>(mem_)
            ) /
            sizeof(StorageT);
        ASSERT_LT(idx, kChunks);
        ASSERT_TRUE(used_map_.Get(idx));

        lock_.Lock();
        used_map_.Set(idx, false);
        ++free_slots_;
        lock_.Unlock();
    }

    FORCE_INLINE_F constexpr void DeallocateAll()
    {
        lock_.Lock();
        used_map_.SetAll(false);
        free_slots_ = kChunks;
        cursor_     = 0;
        lock_.Unlock();
    }

    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock block) const
    {
        const size_t idx = static_cast<size_t>(
                               static_cast<byte *>(block.ptr) - reinterpret_cast<const byte *>(mem_)
                           ) /
                           sizeof(StorageT);

        return (idx < kChunks) ? used_map_.Get(idx) : false;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    BitArray<kChunks> used_map_{};
    StorageT mem_[kChunks]{};
    Spinlock lock_{};
    size_t free_slots_ = kChunks;
    size_t cursor_{};
};

static_assert(
    Allocator<PoolAllocator<32, 64>>,
    "PoolAllocator does not satisfy Allocator concept requirements"
);

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_POOL_ALLOCATOR_HPP_
