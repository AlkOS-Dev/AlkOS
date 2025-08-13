#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_

#include <constants.hpp>
#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/tuple.hpp>

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
    using IntegralType = typename UnsignedIntegral<sizeof(KeyT)>::type;

    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr FastMinimalStaticHashmap() noexcept                                = default;
    constexpr FastMinimalStaticHashmap(const FastMinimalStaticHashmap&) noexcept = default;
    constexpr FastMinimalStaticHashmap(FastMinimalStaticHashmap&&) noexcept      = default;

    constexpr FastMinimalStaticHashmap& operator=(const FastMinimalStaticHashmap&) noexcept =
        default;
    constexpr FastMinimalStaticHashmap& operator=(FastMinimalStaticHashmap&&) noexcept = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    template <bool kOverwrite = false>
    NODISCARD constexpr bool Insert(const KeyT& key, const ValueT& value) noexcept
    {
        ASSERT_LT(
            size_, kSize, "Hashmap overflow attempted, size: %zu, max size: %zu", size_, kSize
        );

        auto [hashed_idx, integral_key] = ConvertKey(key);
        while (keys_[hashed_idx] != 0 && keys_[hashed_idx] != integral_key) {
            hashed_idx = (hashed_idx + 1) % kAdjustedSize;  // Linear probing
        }

        if (keys_[hashed_idx] == integral_key) {
            if constexpr (kOverwrite) {
                values_[hashed_idx] = value;
            }
            return false;
        }

        values_[hashed_idx] = value;
        keys_[hashed_idx]   = integral_key;
        ++size_;
        return true;
    }

    NODISCARD constexpr bool Remove(const KeyT& key) noexcept
    {
        const auto [hashed_idx, integral_key] = ConvertKey(key);

        size_t hash_iterator = hashed_idx;
        size_t counted       = 0;
        while (keys_[hash_iterator] != integral_key && counted < size_) {
            counted++;
            hash_iterator = (hash_iterator + 1) % kAdjustedSize;  // Linear probing
        }

        if (counted == size_) {
            return false;  // Key not found
        }

        TODO_OPTIMISE
        // TODO: Rehash the next elements to fill the gap??

        keys_[hash_iterator] = 0;
        --size_;
        return true;
    }

    NODISCARD FAST_CALL size_t HashKey(const KeyT& key) noexcept
    {
        static constexpr size_t offset = 2166136261U;
        static constexpr size_t prime  = 16777619U;

        IntegralType hash = *reinterpret_cast<const IntegralType*>(&key);
        return ((offset ^ hash) * prime) % kAdjustedSize;
    }

    // ------------------------------
    // Implementation details
    // ------------------------------

    protected:
    NODISCARD FAST_CALL std::tuple<size_t, IntegralType> ConvertKey(const KeyT& key)
    {
        size_t hashed_idx               = HashKey(key);
        const IntegralType integral_key = *reinterpret_cast<const IntegralType*>(&key);
        ASSERT_NOT_ZERO(integral_key, "Zero key is not allowed in FastMinimalStaticHashmap");

        return {hashed_idx, integral_key};
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t kAdjustedSize = std::bit_ceil(kSize) * 2;
    // align to power of two and allow for bigger size to minimize collisions

    alignas(arch::kCacheLineSizeBytes) ValueT values_[kAdjustedSize]{};
    alignas(arch::kCacheLineSizeBytes) IntegralType keys_[kAdjustedSize]{};
    size_t size_{};
};

}  // namespace data_structures

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_
