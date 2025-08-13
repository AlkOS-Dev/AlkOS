#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_

#include <extensions/bit.hpp>
#include <extensions/data_structures/array_structures.hpp>

namespace data_structures
{

// ------------------------------
// FastMinimalStaticHashmap
// ------------------------------

template <class KeyT, class ValueT, size_t kSize>
    requires(IsIntegralSize(sizeof(KeyT)) && kSize > 0 && kSize < 64)
// For bigger size this becomes a performance bottleneck, should be used for
// small key sets only
class FastMinimalStaticHashmap
{
    public:
    using HashType = typename UnsignedIntegral<sizeof(KeyT)>::type;

    // ------------------------------
    // Class creation
    // ------------------------------

    // ------------------------------
    // Class methods
    // ------------------------------

    NODISCARD FAST_CALL size_t HashKey(const KeyT& key) noexcept
    {
        static constexpr size_t offset = 2166136261U;
        static constexpr size_t prime  = 16777619U;

        HashType hash = *reinterpret_cast<const HashType*>(&key);
        return ((offset ^ hash) * prime) % kAdjustedSize;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    static constexpr size_t kAdjustedSize = std::bit_ceil(kSize) * 2;
    // align to power of two and allow for bigger size to minimize collisions

    ValueT values_[kSize]{};
    ArraySingleTypeStaticStack<u8, kSize> free_indices_{};
    HashType keys_[kAdjustedSize]{};
};

}  // namespace data_structures

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_
