#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_THRESHOLD_ADAPTER_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_THRESHOLD_ADAPTER_HPP_

template <size_t kThreshold, Allocator AllocatorLE, Allocator AllocatorGT>
class ThresholdAdapter : private AllocatorLE, private AllocatorGT
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    ThresholdAdapter()  = default;
    ~ThresholdAdapter() = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(size_t n)
    {
        if (n <= kThreshold) {
            return AllocatorLE::Allocate(n);
        } else {
            return AllocatorGT::Allocate(n);
        }
    }

    constexpr void Deallocate(AllocatorBlock block)
    {
        if (AllocatorLE::Owns(block)) {
            AllocatorLE::Deallocate(block);
        } else {
            AllocatorGT::Deallocate(block);
        }
    }

    FORCE_INLINE_F constexpr void DeallocateAll()
    {
        AllocatorLE::DeallocateAll();
        AllocatorGT::DeallocateAll();
    }

    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock block) const
    {
        return AllocatorLE::Owns(block) || AllocatorGT::Owns(block);
    }
};

#include <extensions/internal/allocators/stub_allocator.hpp>

static_assert(
    Allocator<ThresholdAdapter<0, StubAllocator<0>, StubAllocator<1>>>,
    "ThresholdAdapter must be an Allocator"
);

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_THRESHOLD_ADAPTER_HPP_
