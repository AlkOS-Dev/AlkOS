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
    using IntegralType                = typename UnsignedIntegral<sizeof(KeyT)>::type;
    static constexpr size_t kNotFound = kFullMask<size_t>;

    // ------------------------------
    // Class creation
    // ------------------------------

    FastMinimalStaticHashmap() noexcept                                = default;
    FastMinimalStaticHashmap(const FastMinimalStaticHashmap&) noexcept = default;
    FastMinimalStaticHashmap(FastMinimalStaticHashmap&&) noexcept      = default;

    FastMinimalStaticHashmap& operator=(const FastMinimalStaticHashmap&) noexcept = default;
    FastMinimalStaticHashmap& operator=(FastMinimalStaticHashmap&&) noexcept      = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    template <bool kOverwrite = false>
    bool Insert(const KeyT& key, const ValueT& value)
    {
        ASSERT_LT(
            size_, kSize, "Hashmap overflow attempted, size: %zu, max size: %zu", size_, kSize
        );

        auto [hashed_idx, integral_key] = ConvertKey_(key);
        while (keys_[hashed_idx] != 0 && keys_[hashed_idx] != integral_key) {
            hashed_idx = (hashed_idx + 1) % kAdjustedSize;  // Linear probing
        }

        if (keys_[hashed_idx] == integral_key) {
            if constexpr (kOverwrite) {
                *GetTypePtr_(hashed_idx) = value;
            }
            return false;
        }

        *GetTypePtr_(hashed_idx) = value;
        keys_[hashed_idx]        = integral_key;
        ++size_;
        return true;
    }

    template <class... Args>
    bool Emplace(const KeyT& key, Args&&... args)
    {
        ASSERT_LT(
            size_, kSize, "Hashmap overflow attempted, size: %zu, max size: %zu", size_, kSize
        );

        auto [hashed_idx, integral_key] = ConvertKey_(key);
        while (keys_[hashed_idx] != 0 && keys_[hashed_idx] != integral_key) {
            hashed_idx = (hashed_idx + 1) % kAdjustedSize;  // Linear probing
        }

        if (keys_[hashed_idx] == integral_key) {
            return false;
        }

        new (GetTypePtr_(hashed_idx)) ValueT(std::forward<Args>(args)...);
        keys_[hashed_idx] = integral_key;
        ++size_;
        return true;
    }

    bool Remove(const KeyT& key)
    {
        const auto idx = FindIndex_(key);

        if (idx == kNotFound) {
            return false;  // Key not found
        }

        TODO_OPTIMISE
        // TODO: Rehash the next elements to fill the gap??

        keys_[idx] = 0;
        --size_;
        GetTypePtr_(idx)->~ValueT();

        return true;
    }

    NODISCARD ValueT* Find(const KeyT& key) const
    {
        if (size_ == 0) {
            return nullptr;
        }

        const auto idx = FindIndex_(key);
        if (idx == kNotFound) {
            return nullptr;  // Key not found
        }

        TODO_OPTIMISE
        // TODO: If rehashed first gaps is stop signal?

        return GetTypePtr_(idx);
    }

    NODISCARD bool HasKey(const KeyT& key) const
    {
        if (size_ == 0) {
            return false;
        }

        const auto idx = FindIndex_(key);
        if (idx == kNotFound) {
            return false;  // Key not found
        }

        return true;
    }

    NODISCARD FAST_CALL size_t HashKey(const KeyT& key)
    {
        static constexpr size_t offset = 2166136261U;
        static constexpr size_t prime  = 16777619U;

        IntegralType hash = *reinterpret_cast<const IntegralType*>(&key);
        return ((offset ^ hash) * prime) % kAdjustedSize;
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return size_; }

    // ------------------------------
    // Implementation details
    // ------------------------------

    protected:
    NODISCARD FAST_CALL std::tuple<size_t, IntegralType> ConvertKey_(const KeyT& key)
    {
        size_t hashed_idx               = HashKey(key);
        const IntegralType integral_key = *reinterpret_cast<const IntegralType*>(&key);
        ASSERT_NOT_ZERO(integral_key, "Zero key is not allowed in FastMinimalStaticHashmap");

        return {hashed_idx, integral_key};
    }

    NODISCARD FORCE_INLINE_F size_t FindIndex_(const KeyT& key) const
    {
        const auto [hashed_idx, integral_key] = ConvertKey_(key);

        size_t hash_iterator = hashed_idx;
        size_t counted       = 0;
        while (keys_[hash_iterator] != integral_key && counted < size_) {
            counted += (keys_[hash_iterator] != 0);               // Count only non-zero keys
            hash_iterator = (hash_iterator + 1) % kAdjustedSize;  // Linear probing
        }

        if (counted == size_) {
            return kNotFound;  // Key not found
        }

        return hash_iterator;
    }

    NODISCARD FORCE_INLINE_F ValueT* GetTypePtr_(const size_t idx) const
    {
        return reinterpret_cast<ValueT*>(values_->data[sizeof(ValueT) * idx]);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t kAdjustedSize = std::bit_ceil(kSize) * 2;
    // align to power of two and allow for bigger size to minimize collisions

    alignas(arch::kCacheLineSizeBytes) std::aligned_storage_t<
        sizeof(ValueT), alignof(ValueT)> values_[kAdjustedSize];  // No need to initialize

    alignas(arch::kCacheLineSizeBytes) IntegralType keys_[kAdjustedSize]{};
    size_t size_{};
};

// ------------------------------
// Registry
// ------------------------------

struct RegistryEntry {
    RegistryEntry() = default;
    u64 id;
};

template <class T, size_t kSize>
    requires(kSize < 64 && std::is_base_of_v<RegistryEntry, T> && std::is_trivially_copyable_v<T>)
class Registry
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Registry()  = default;
    ~Registry() = default;

    // ------------------------------
    // Data access
    // ------------------------------

    FORCE_INLINE_F u64* begin() { return key_vector_.begin(); }
    FORCE_INLINE_F u64* end() { return key_vector_.end(); }
    FORCE_INLINE_F const u64* cbegin() const { return key_vector_.cbegin(); }
    FORCE_INLINE_F const u64* cend() const { return key_vector_.cend(); }
    FORCE_INLINE_F T& operator[](const u64 key) { return *entries_.Find(key); }
    FORCE_INLINE_F const T& operator[](const u64 key) const { return *entries_.Find(key); }

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F void Register(const T& entry)
    {
        [[maybe_unused]] const bool result = entries_.Insert(entry.id, entry);
        ASSERT_TRUE(result, "Tried to register item twice with id: %llu", entry.id);
        key_vector_.Push(entry.id);
    }

    template <class... Args>
    FORCE_INLINE_F void RegisterEmplace(const u64 id, Args&&... args)
    {
        [[maybe_unused]] const bool result = entries_.Emplace(id, std::forward<Args>(args)...);
        ASSERT_TRUE(result, "Tried to register item twice with id: %llu", id);
        key_vector_.Push(id);
    }

    NODISCARD FORCE_INLINE_F bool IsActivePicked() const { return is_active_; }

    NODISCARD FORCE_INLINE_F bool HasKey(const u64 key) const { return entries_.HasKey(key); }

    NODISCARD FORCE_INLINE_F T& GetActive()
    {
        ASSERT_TRUE(IsActivePicked(), "Tried to get active item, but no active item is set!");
        return active_;
    }

    NODISCARD FORCE_INLINE_F const T& GetActive() const
    {
        ASSERT_TRUE(IsActivePicked(), "Tried to get active item, but no active item is set!");
        return active_;
    }

    FORCE_INLINE_F void SetActive(const u64 key)
    {
        ASSERT_TRUE(
            entries_.HasKey(key), "Tried to set active item with non-existing key: %llu", key
        );
        active_    = *entries_.Find(key);
        is_active_ = true;
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return entries_.Size(); }

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    FastMinimalStaticHashmap<u64, T, kSize> entries_{};
    StaticVector<u64, kSize> key_vector_{};
    T active_;  // Uninitialized intentionally
    bool is_active_{false};
};

}  // namespace data_structures

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_HASH_MAPS_HPP_
