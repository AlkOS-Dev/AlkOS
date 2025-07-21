#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_AFFIX_ALLOCATOR_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_AFFIX_ALLOCATOR_HPP_

#include <extensions/concepts.hpp>
#include <extensions/internal/allocators/allocator_base.hpp>

template <Allocator T, typename Prefix, typename Suffix = void>
class AffixAllocator : private T
{
    public:
    using wrapped_type = T;
    using prefix_type  = Prefix;
    using suffix_type  = Suffix;

    static constexpr size_t prefix_size =
        (std::same_as<prefix_type, void>) ? 0 : sizeof(prefix_type);
    static constexpr size_t suffix_size =
        (std::same_as<suffix_type, void>) ? 0 : sizeof(suffix_type);

    // ------------------------------
    // Class creation
    // ------------------------------

    AffixAllocator()  = default;
    ~AffixAllocator() = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD constexpr AllocatorBlock Allocate(size_t n)
    {
        if (n == 0) {
            return nullptr;
        }

        // Allocate memory for the prefix, main object, and suffix
        size_t total_size = prefix_size + n + suffix_size;
        auto block        = T::Allocate(total_size);
        block.size        = n;

        // Construct prefix and suffix objects
        if constexpr (prefix_size > 0) {
            new (block.ptr) prefix_type();
        }
        if constexpr (suffix_size > 0) {
            new (static_cast<byte *>(block.ptr) + prefix_size + n) suffix_type();
        }
        block.ptr = static_cast<byte *>(block.ptr) + prefix_size;

        return block;
    }

    constexpr void Deallocate(AllocatorBlock block)
    {
        if (block.ptr == nullptr || block.size == 0) {
            return;
        }

        // Deconstruct prefix and suffix objects
        if constexpr (prefix_size > 0 && !std::is_trivially_destructible_v<prefix_type>) {
            reinterpret_cast<prefix_type *>(static_cast<byte *>(block.ptr) - prefix_size)
                ->~prefix_type();
        }
        if constexpr (suffix_size > 0 && !std::is_trivially_destructible_v<suffix_type>) {
            reinterpret_cast<suffix_type *>(static_cast<byte *>(block.ptr) + block.size)
                ->~suffix_type();
        }

        // Deallocate the entire block
        T::Deallocate(
            {static_cast<byte *>(block.ptr) - prefix_size, block.size + prefix_size + suffix_size}
        );
    }

    FORCE_INLINE_F constexpr void DeallocateAll() { T::DeallocateAll(); }

    NODISCARD FORCE_INLINE_F constexpr bool Owns(AllocatorBlock block) const
    {
        return T::Owns(
            {static_cast<byte *>(block.ptr) - prefix_size, block.size + prefix_size + suffix_size}
        );
    }
};

#include <extensions/internal/allocators/stub_allocator.hpp>

static_assert(
    Allocator<AffixAllocator<StubAllocator<>, void>>, "AffixAllocator must be an Allocator"
);

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_ALLOCATORS_AFFIX_ALLOCATOR_HPP_
