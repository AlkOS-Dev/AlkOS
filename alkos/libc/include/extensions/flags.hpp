#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_FLAGS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_FLAGS_HPP_

#include <extensions/bit_array.hpp>
#include <extensions/template_lib.hpp>

#define __DEF_FEATURE_FLAGS(name, ...)                                     \
    enum class name : size_t { __VA_ARGS__ };                              \
    using name##_t = make_flags<name>;                                     \
    NODISCARD constexpr name##_t operator|(const name lhs, const name rhs) \
    {                                                                      \
        return name##_t({lhs, rhs});                                       \
    }

namespace flags
{
namespace internal
{
template <typename EnumT>
    requires std::is_enum_v<EnumT>
struct FeatureFlags {
    // ------------------------------
    // Member types
    // ------------------------------

    using value_type                    = EnumT;
    using underlying_type               = std::underlying_type_t<value_type>;
    static constexpr size_t kFlagsCount = template_lib::EnumCount_v<EnumT>;

    private:
    using StorageT                        = u32;
    static constexpr size_t kStorageTBits = sizeof(StorageT) * 8;
    static constexpr size_t kNumStorageT  = (kFlagsCount + kStorageTBits - 1) / kStorageTBits;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr FeatureFlags() = default;
    constexpr FeatureFlags(EnumT flag) { EnableFlag(flag); }
    constexpr FeatureFlags(const std::initializer_list<EnumT>& flags)
    {
        for (const auto& flag : flags) {
            EnableFlag(flag);
        }
    }

    // ------------------------------
    // Operators
    // ------------------------------

    NODISCARD constexpr bool operator[](const EnumT f) const { return TestFlag(f); }
    constexpr FeatureFlags& operator|(const EnumT f)
    {
        EnableFlag(f);
        return *this;
    }
    NODISCARD constexpr bool operator&(const EnumT f) const { return TestFlag(f); }

    // ------------------------------
    // Methods
    // ------------------------------

    private:
    FORCE_INLINE_F constexpr void SetTrue(const size_t index)
    {
        if (!std::is_constant_evaluated()) {
            ASSERT_LT(index, kFlagsCount, "Index out of bounds in FeatureFlags");
        }

        _storage_[index / kStorageTBits] |= (kLsb<StorageT> << (index % kStorageTBits));
    }

    FORCE_INLINE_F constexpr void SetFalse(const size_t index)
    {
        if (!std::is_constant_evaluated()) {
            ASSERT_LT(index, kFlagsCount, "Index out of bounds in FeatureFlags");
        }

        _storage_[index / kStorageTBits] &= ~(kLsb<StorageT> << (index % kStorageTBits));
    }

    NODISCARD FORCE_INLINE_F constexpr bool Get(const size_t index) const
    {
        if (!std::is_constant_evaluated()) {
            ASSERT_LT(index, kFlagsCount, "Index out of bounds in FeatureFlags");
        }

        return (_storage_[index / kStorageTBits] >> (index % kStorageTBits)) & kLsb<StorageT>;
    }

    public:
    constexpr void EnableFlag(const EnumT f) { SetTrue(static_cast<underlying_type>(f)); }
    constexpr void DisableFlag(const EnumT f) { SetFalse(static_cast<underlying_type>(f)); }
    NODISCARD constexpr bool TestFlag(const EnumT f) const
    {
        return Get(static_cast<underlying_type>(f));
    }

    // ------------------------------
    // Data members
    // ------------------------------

    StorageT _storage_[kNumStorageT]{};
};

}  // namespace internal

template <typename EnumT>
using make_flags = internal::FeatureFlags<EnumT>;

__DEF_FEATURE_FLAGS(AllocatorStats, Allocations, Deallocations, DebugInfo)

}  // namespace flags

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_FLAGS_HPP_
