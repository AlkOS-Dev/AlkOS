#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_BITFIELD_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_BITFIELD_HPP_

#include <stddef.h>
#include <bit.hpp>
#include <data_structures/bit_array.hpp>
#include <internal/macro.hpp>
#include <tuple.hpp>
#include <type_traits.hpp>

namespace data_structures::internal
{

// Type-safe field tag
template <typename Owner, std::size_t Index, std::size_t Width>
struct BitFieldTag {
    using owner_type                   = Owner;
    static constexpr std::size_t index = Index;
    static constexpr std::size_t width = Width;
};

// BitField specification
template <typename Tag>
struct BitFieldSpec {
    using tag_type                     = Tag;
    static constexpr std::size_t width = Tag::width;
    static constexpr std::size_t index = Tag::index;
};

// Calculate offset for a field at Index
template <std::size_t Index, typename... Specs>
struct OffsetCalculator;

template <std::size_t Index, typename First, typename... Rest>
struct OffsetCalculator<Index, First, Rest...> {
    static constexpr std::size_t value =
        (Index == First::index) ? 0 : First::width + OffsetCalculator<Index, Rest...>::value;
};

template <std::size_t Index, typename First>
struct OffsetCalculator<Index, First> {
    static constexpr std::size_t value = (Index == First::index) ? 0 : First::width;
};

// Calculate total size in bits
template <typename... Specs>
struct TotalSize;

template <typename First, typename... Rest>
struct TotalSize<First, Rest...> {
    static constexpr std::size_t value = First::width + TotalSize<Rest...>::value;
};

template <>
struct TotalSize<> {
    static constexpr std::size_t value = 0;
};

// Find spec by tag index
template <std::size_t Index, typename... Specs>
struct FindSpecByIndex;

template <std::size_t Index, typename First, typename... Rest>
struct FindSpecByIndex<Index, First, Rest...> {
    static constexpr bool found = (First::index == Index);
    using type = std::conditional_t<found, First, typename FindSpecByIndex<Index, Rest...>::type>;
};

template <std::size_t Index>
struct FindSpecByIndex<Index> {
    static constexpr bool found = false;
    using type                  = void;
};

template <typename T>
struct IsBitField : std::false_type {
};

template <typename Derived, typename... FieldSpecs>
class EnumBitFieldsBase;

template <typename Derived, typename... FieldSpecs>
struct IsBitField<EnumBitFieldsBase<Derived, FieldSpecs...>> : std::true_type {
};

template <typename T>
concept NotBitField = !IsBitField<std::remove_cvref_t<T>>::value;

template <typename Derived, typename... FieldSpecs>
class EnumBitFieldsBase
{
    public:
    static constexpr std::size_t kTotalBits = TotalSize<FieldSpecs...>::value;

    protected:
    BitArray<kTotalBits> data_;

    template <typename T, std::size_t Offset, std::size_t Width>
    static constexpr T extract_bits(const BitArray<kTotalBits> &arr)
    {
        return arr.template GetRange<T, Offset, Width>();
    }

    template <typename T, std::size_t Offset, std::size_t Width>
    static constexpr void insert_bits(BitArray<kTotalBits> &arr, T value)
    {
        arr.template SetRange<T, Offset, Width>(value);
    }

    public:
    constexpr EnumBitFieldsBase() = default;

    constexpr EnumBitFieldsBase(const EnumBitFieldsBase &) = default;

    template <typename OtherDerived, typename... OtherSpecs>
    constexpr EnumBitFieldsBase(const EnumBitFieldsBase<OtherDerived, OtherSpecs...> &other)
        : data_(other.storage())
    {
        static_assert(
            kTotalBits == EnumBitFieldsBase<OtherDerived, OtherSpecs...>::kTotalBits,
            "BitField construction failed: Source and Destination must have the same total size in "
            "bits."
        );
    }

    template <NotBitField T>
    constexpr explicit EnumBitFieldsBase(const T &value)
    {
        static_assert(
            sizeof(T) == data_.kNumStorage,
            "Raw Type construction failed: Size of input type must match BitField storage size."
        );

        data_ = std::bit_cast<BitArray<kTotalBits>>(value);
    }

    template <NotBitField T>
    constexpr EnumBitFieldsBase &operator=(const T &value)
    {
        static_assert(
            sizeof(T) == data_.kNumStorage,
            "Raw Type assignment failed: Size of input type must match BitField storage size."
        );

        data_ = std::bit_cast<BitArray<kTotalBits>>(value);
        return *this;
    }

    template <NotBitField T>
    constexpr explicit operator T() const
    {
        static_assert(
            sizeof(T) == data_.kNumStorage,
            "Cast to Raw Type failed: Size of destination type must match BitField storage size."
        );

        return std::bit_cast<T>(data_);
    }

    template <typename OtherDerived, typename... OtherSpecs>
    constexpr EnumBitFieldsBase &operator=(
        const EnumBitFieldsBase<OtherDerived, OtherSpecs...> &other
    )
    {
        static_assert(
            kTotalBits == EnumBitFieldsBase<OtherDerived, OtherSpecs...>::kTotalBits,
            "BitField assignment failed: Source and Destination bitfields must have the same total "
            "size in bits."
        );

        this->data_ = other.storage();
        return *this;
    }

    template <std::size_t Index>
    static constexpr std::size_t offset_for()
    {
        return OffsetCalculator<Index, FieldSpecs...>::value;
    }

    template <typename Tag>
    constexpr auto get() const
    {
        static_assert(std::is_same_v<typename Tag::owner_type, Derived>, "Invalid Tag used");
        using Spec                   = typename FindSpecByIndex<Tag::index, FieldSpecs...>::type;
        constexpr std::size_t offset = offset_for<Tag::index>();
        constexpr std::size_t width  = Spec::width;
        return extract_bits<std::size_t, offset, width>(data_);
    }

    template <typename Tag>
    constexpr void set(std::size_t value)
    {
        static_assert(std::is_same_v<typename Tag::owner_type, Derived>, "Invalid Tag used");
        using Spec                   = typename FindSpecByIndex<Tag::index, FieldSpecs...>::type;
        constexpr std::size_t offset = offset_for<Tag::index>();
        constexpr std::size_t width  = Spec::width;
        insert_bits<std::size_t, offset, width>(data_, value);
    }

    constexpr const BitArray<kTotalBits> &storage() const { return data_; }
    constexpr BitArray<kTotalBits> &storage() { return data_; }
};

}  // namespace data_structures::internal

#define BF_ENUM_ENTRY(name, width) k##name

#define BF_FIELD_TAG(ctx, name, width)                   \
    using name = data_structures::internal::BitFieldTag< \
        ctx::Owner, static_cast<std::size_t>(ctx::Index::k##name), width>

#define BF_FIELD_SPEC(ctx, name, width)                                             \
    data_structures::internal::BitFieldSpec<data_structures::internal::BitFieldTag< \
        ctx::Owner, static_cast<std::size_t>(ctx::Index::k##name), width>>

#define CREATE_BITFIELD(ClassName, ...)                                                      \
    class ClassName;                                                                         \
                                                                                             \
    namespace ClassName##Details                                                             \
    {                                                                                        \
        using Owner = ClassName;                                                             \
        enum class Index : std::size_t { FOR_EACH_PAIR(BF_ENUM_ENTRY, __VA_ARGS__), kLast }; \
    }                                                                                        \
                                                                                             \
    class ClassName                                                                          \
        : public data_structures::internal::EnumBitFieldsBase<                               \
              ClassName, FOR_EACH_PAIR_CTX(BF_FIELD_SPEC, ClassName##Details, __VA_ARGS__)>  \
    {                                                                                        \
        private:                                                                             \
        using Base = data_structures::internal::EnumBitFieldsBase<                           \
            ClassName, FOR_EACH_PAIR_CTX(BF_FIELD_SPEC, ClassName##Details, __VA_ARGS__)>;   \
                                                                                             \
        public:                                                                              \
        using Base::Base;                                                                    \
        using Base::operator=;                                                               \
        using Base::get;                                                                     \
        using Base::set;                                                                     \
                                                                                             \
        FOR_EACH_PAIR_CTX_SEP(SEMICOLON, BF_FIELD_TAG, ClassName##Details, __VA_ARGS__);     \
    }

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_BITFIELD_HPP_
