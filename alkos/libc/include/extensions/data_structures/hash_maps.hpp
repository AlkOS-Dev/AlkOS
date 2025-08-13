#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_

namespace data_structures
{

// ------------------------------
// FastMinimalStaticHashmap
// ------------------------------

template <class KeyT, class ValueT, size_t kSize>
    requires(sizeof(KeyT) <= 8 && kSize > 0 && kSize < 64)
// For bigger size this becomes a performance bottleneck, should be used for
// small key sets only
class FastMinimalStaticHashmap
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    // ------------------------------
    // Class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
};

}  // namespace data_structures

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_
