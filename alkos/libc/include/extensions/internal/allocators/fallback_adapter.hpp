#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_FALLBACK_ADAPTER_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_FALLBACK_ADAPTER_HPP_

#include <extensions/internal/allocators/allocator_base.hpp>

template <Allocator PrimaryAllocator, Allocator FallbackAllocator>
class FallbackAdapter : private PrimaryAllocator, private FallbackAllocator
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    FallbackAdapter()  = default;
    ~FallbackAdapter() = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(size_t n)
    {
        auto block = PrimaryAllocator::Allocate(n);
        if (!block) {
            block = FallbackAllocator::Allocate(n);
        }
        return block;
    }

    constexpr void Deallocate(AllocatorBlock block)
    {
        if (PrimaryAllocator::Owns(block)) {
            PrimaryAllocator::Deallocate(block);
        } else {
            FallbackAllocator::Deallocate(block);
        }
    }

    FORCE_INLINE_F constexpr void DeallocateAll()
    {
        PrimaryAllocator::DeallocateAll();
        FallbackAllocator::DeallocateAll();
    }

    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock block) const
    {
        return PrimaryAllocator::Owns(block) || FallbackAllocator::Owns(block);
    }
};

#include <extensions/internal/allocators/stub_allocator.hpp>

static_assert(
    Allocator<FallbackAdapter<StubAllocator<0>, StubAllocator<1>>>,
    "FallbackAdapter must be an Allocator"
);

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_FALLBACK_ADAPTER_HPP_
