#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_STACK_ALLOCATOR_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_STACK_ALLOCATOR_HPP_

#include <extensions/bit.hpp>
#include <extensions/internal/allocators/allocator_base.hpp>

template <size_t kStackSize, size_t kAlignment = alignof(std::max_align_t)>
class StackAllocator
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    StackAllocator() = default;
    ~StackAllocator() { ASSERT_EQ(top_, stack_memory_, "Memory leak in StackAllocator"); }

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(size_t n)
    {
        size_t aligned_size = AlignUp(n, kAlignment);
        if (aligned_size > GetFreeSpace()) {
            return {nullptr, 0};
        }

        AllocatorBlock block = {top_, aligned_size};
        top_ += aligned_size;
        return block;
    }

    constexpr void Deallocate(AllocatorBlock block)
    {
        size_t aligned_size = AlignUp(block.size, kAlignment);
        if (static_cast<byte *>(block.ptr) + aligned_size == top_) {
            top_ = static_cast<byte *>(block.ptr);
        } else {
            FAIL_ALWAYS("Deallocation of non-top block in StackAllocator is not allowed");
        }
    }

    FORCE_INLINE_F constexpr void DeallocateAll() { top_ = stack_memory_; }

    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock block) const
    {
        return (block.ptr >= stack_memory_) && (block.ptr < stack_memory_ + kStackSize);
    }

    NODISCARD FORCE_INLINE_F constexpr size_t GetCurrentPosition() const
    {
        return top_ - stack_memory_;
    }

    FORCE_INLINE_F constexpr void SetPosition(size_t pos)
    {
        ASSERT_LT(pos, kStackSize, "Position out of bounds in StackAllocator");
        top_ = stack_memory_ + pos;
    }

    private:
    // ------------------------------
    // Helper methods
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_t GetFreeSpace() const
    {
        return stack_memory_ + kStackSize - top_;
    }

    // ------------------------------
    // Data members
    // ------------------------------

    alignas(kAlignment) byte stack_memory_[kStackSize];
    byte *top_ = stack_memory_;
};

static_assert(Allocator<StackAllocator<0>>, "StackAllocator must satisfy Allocator concept");

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_STACK_ALLOCATOR_HPP_
