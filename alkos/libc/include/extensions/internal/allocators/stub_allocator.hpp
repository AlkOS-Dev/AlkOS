#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_STUB_ALLOCATOR_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_STUB_ALLOCATOR_HPP_

#include <extensions/internal/allocators/allocator_base.hpp>

template <size_t N = 0>
class StubAllocator
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    StubAllocator()  = default;
    ~StubAllocator() = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(size_t) { return {}; }
    constexpr void Deallocate(AllocatorBlock) {}
    FORCE_INLINE_F constexpr void DeallocateAll() {}
    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock) const { return false; }
};

static_assert(Allocator<StubAllocator<>>, "StubAllocator should satisfy the Allocator concept");

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_STUB_ALLOCATOR_HPP_
