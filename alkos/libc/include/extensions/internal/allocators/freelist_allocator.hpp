#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_FREELIST_ALLOCATOR_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_FREELIST_ALLOCATOR_HPP_

#include <extensions/internal/allocators/allocator_base.hpp>

template <Allocator T, size_t kMinBlockSize, size_t kMaxBlockSize, size_t kMaxNumBlocks>
class FreeListAllocator : private T
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    FreeListAllocator() = default;
    ~FreeListAllocator()
    {
        ASSERT_EQ(total_blocks_, free_blocks_, "FreeListAllocator: Memory leak detected");
        DeallocateAll();
    }

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(size_t n)
    {
        if (IsValidBlockSize(n) && root_) {
            --free_blocks_;

            AllocatorBlock block = {root_, kMaxBlockSize};
            root_                = root_->next;
            return block;
        }

        return T::Allocate(kMaxBlockSize);
    }

    constexpr void Deallocate(AllocatorBlock block)
    {
        if (block.ptr == nullptr || block.size == 0) {
            return;
        }

        if (!IsValidBlockSize(block.size) || total_blocks_ > kMaxNumBlocks) {
            T::Deallocate(block);
            return;
        }

        if (free_blocks_ == total_blocks_) {
            ++total_blocks_;
        }
        ++free_blocks_;

        Node* node = static_cast<Node*>(block.ptr);
        node->next = root_;
        root_      = node;
    }

    FORCE_INLINE_F constexpr void DeallocateAll() { T::DeallocateAll(); }

    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock block) const
    {
        return IsValidBlockSize(block.size) || T::Owns(block);
    }

    private:
    NODISCARD FORCE_INLINE_F constexpr size_t IsValidBlockSize(size_t n) const
    {
        return n >= kMinBlockSize && n <= kMaxBlockSize;
    }

    struct Node {
        Node* next;
    }* root_{};
    size_t total_blocks_{};
    size_t free_blocks_{};
};

#include <extensions/internal/allocators/stub_allocator.hpp>

static_assert(
    Allocator<FreeListAllocator<StubAllocator<0>, 16, 32, 1024>>,
    "FreeListAllocator must be an Allocator"
);

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_FREELIST_ALLOCATOR_HPP_
