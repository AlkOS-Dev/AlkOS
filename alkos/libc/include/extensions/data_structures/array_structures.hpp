#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_ARRAY_STRUCTURES_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_ARRAY_STRUCTURES_HPP_

#include <string.h>
#include <extensions/array.hpp>
#include <extensions/concepts.hpp>
#include <extensions/debug.hpp>

namespace data_structures
{
// ------------------------------
// StaticPlainMap
// ------------------------------

template <class ItemT, size_t kSize, class KeyT = size_t>
    requires(
        std::convertible_to<KeyT, size_t> && std::is_trivially_copyable_v<ItemT> &&
        std::is_default_constructible_v<ItemT> && kSize > 0 &&
        kSize < 256  // Static maps should not be too large to avoid excessive memory usage
    )
class StaticPlainMapDefaultConstructible
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    constexpr StaticPlainMapDefaultConstructible()  = default;
    constexpr ~StaticPlainMapDefaultConstructible() = default;

    constexpr StaticPlainMapDefaultConstructible(const StaticPlainMapDefaultConstructible &) =
        default;
    constexpr StaticPlainMapDefaultConstructible(StaticPlainMapDefaultConstructible &&) = default;

    constexpr StaticPlainMapDefaultConstructible &operator=(
        const StaticPlainMapDefaultConstructible &
    ) = default;
    constexpr StaticPlainMapDefaultConstructible &operator=(StaticPlainMapDefaultConstructible &&) =
        default;

    // ------------------------------
    // Class methods
    // ------------------------------

    FORCE_INLINE_F constexpr ItemT &operator[](const KeyT &key) noexcept
    {
        const size_t idx = static_cast<size_t>(key);

        ASSERT_LT(idx, kSize, "Index out of range");
        return map_[idx];
    }

    FORCE_INLINE_F constexpr const ItemT &operator[](const KeyT &key) const noexcept
    {
        const size_t idx = static_cast<size_t>(key);

        ASSERT_LT(idx, kSize, "Index out of range");
        return map_[idx];
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    ItemT map_[kSize]{};
};

// ------------------------------
// EnumMap
// ------------------------------

template <class ItemT, class EnumT>
    requires std::is_enum_v<EnumT>
using EnumMap = StaticPlainMapDefaultConstructible<ItemT, static_cast<size_t>(EnumT::kLast), EnumT>;

// ------------------------------
// EnumFlagMap
// ------------------------------

template <class EnumT>
    requires std::is_enum_v<EnumT>
using EnumFlagMap =
    StaticPlainMapDefaultConstructible<bool, static_cast<size_t>(EnumT::kLast), EnumT>;

// ------------------------------
// ArrayStaticStack
// ------------------------------

template <size_t kSizeBytes, size_t kAlignment = 8>
class ArrayStaticStack
{
    // ------------------------------
    // Class creation
    // ------------------------------

    template <class T>
    using clean_type = std::remove_reference_t<std::remove_const_t<T>>;

    public:
    ArrayStaticStack()  = default;
    ~ArrayStaticStack() = default;

    ArrayStaticStack(const ArrayStaticStack &) = default;
    ArrayStaticStack(ArrayStaticStack &&)      = default;

    ArrayStaticStack &operator=(const ArrayStaticStack &) = default;
    ArrayStaticStack &operator=(ArrayStaticStack &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <class T>
    FORCE_INLINE_F T &Push(const T &item)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(item); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T>
    FORCE_INLINE_F T &Push(T &&item)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(std::move(item)); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T, class... Args>
    FORCE_INLINE_F T &PushEmplace(Args &&...args)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(std::forward<Args>(args)...); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T>
    FORCE_INLINE_F std::remove_reference_t<std::remove_const_t<T>> Pop()
    {
        ASSERT_LE(sizeof(T), top_, "Stack underflow!!");
        top_ -= sizeof(T);

        auto ptr = reinterpret_cast<clean_type<T> *>(stack_.data + top_);
        return std::move(*ptr);
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return top_; }

    NODISCARD FORCE_INLINE_F const void *Data() const { return stack_.data; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    template <class T>
    clean_type<T> *PushMem_() noexcept
    {
        const size_t start = top_;
        top_ += sizeof(T);

        ASSERT_LE(top_, kSizeBytes, "Stack overflow!");

        return reinterpret_cast<clean_type<T> *>(stack_.data + start);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    size_t top_{};
    std::aligned_storage_t<kSizeBytes, kAlignment> stack_{};
};

// ------------------------------
// ArraySingleTypeStaticStack
// ------------------------------

template <class T, size_t kNumObjects>
class ArraySingleTypeStaticStack : public ArrayStaticStack<sizeof(T) * kNumObjects, alignof(T)>
{
    using base_t = ArrayStaticStack<sizeof(T) * kNumObjects, alignof(T)>;
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    ArraySingleTypeStaticStack()  = default;
    ~ArraySingleTypeStaticStack() = default;

    ArraySingleTypeStaticStack(const ArraySingleTypeStaticStack &) = default;
    ArraySingleTypeStaticStack(ArraySingleTypeStaticStack &&)      = default;

    ArraySingleTypeStaticStack &operator=(const ArraySingleTypeStaticStack &) = default;
    ArraySingleTypeStaticStack &operator=(ArraySingleTypeStaticStack &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F T &Push(const T &item) { return base_t::Push(item); }

    FORCE_INLINE_F T &Push(T &&item) { return base_t::Push(std::move(item)); }

    template <class... Args>
    FORCE_INLINE_F T &PushEmplace(Args &&...args)
    {
        return base_t::template PushEmplace<T>(std::forward<Args>(args)...);
    }

    FORCE_INLINE_F T Pop() { return std::move(base_t::template Pop<T>()); }

    FORCE_INLINE_F size_t Size() const { return base_t::Size() / sizeof(T); }

    FORCE_INLINE_F size_t SizeBytes() const { return base_t::Size(); }

    using base_t::Data;
};

// ------------------------------
// Static Vector
// ------------------------------

template <class T, size_t kMaxObjects>
class StaticVector : public ArraySingleTypeStaticStack<T, kMaxObjects>
{
    using base_t = ArraySingleTypeStaticStack<T, kMaxObjects>;

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    StaticVector()  = default;
    ~StaticVector() = default;

    StaticVector(const StaticVector &) = default;
    StaticVector(StaticVector &&)      = default;

    StaticVector &operator=(const StaticVector &) = default;
    StaticVector &operator=(StaticVector &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    using base_t::Pop;
    using base_t::Push;
    using base_t::PushEmplace;
    using base_t::Size;

    // Iterator support
    FORCE_INLINE_F T *begin() { return reinterpret_cast<T *>(this->stack_.data); }
    FORCE_INLINE_F T *end() { return reinterpret_cast<T *>(this->stack_.data) + Size(); }

    FORCE_INLINE_F const T *cbegin() const
    {
        return reinterpret_cast<const T *>(this->stack_.data);
    }
    FORCE_INLINE_F const T *cend() const
    {
        return reinterpret_cast<const T *>(this->stack_.data) + Size();
    }

    FORCE_INLINE_F T &operator[](const size_t idx)
    {
        ASSERT_LT(idx, Size(), "Overflow detected!");
        return (begin()[idx]);
    }

    FORCE_INLINE_F const T &operator[](const size_t idx) const
    {
        ASSERT_LT(idx, Size(), "Overflow detected!");
        return (begin()[idx]);
    }

    void Resize(const size_t size)
    {
        ASSERT_LE(size, kMaxObjects, "Too many objects requested!!");

        if (size == Size()) {
            return;
        }

        if (size < Size()) {
            for (auto iter = begin() + size; iter != end(); ++iter) {
                iter->~T();
            }
        } else {
            for (auto iter = begin() + Size(); iter != begin() + size; ++iter) {
                new (iter) T();
            }
        }
        this->top_ = size * sizeof(T);
    }
};

// ------------------------------
// StaticString
// ------------------------------

/*
 * A static string implementation that can hold a fixed-size string.
 * It is not null-terminated, so it should be used with care.
 * Use GetSafeStr() to get a null-terminated version of the string.
 */
template <size_t kSize>
struct StringArray : public std::array<char, kSize> {
    constexpr StringArray(const char *str) noexcept
    {
        for (size_t i = 0; i < kSize && str[i] != '\0'; ++i) {
            this->at(i) = str[i];
        }
    }

    std::array<char, kSize + 1> GetSafeStr() const noexcept
    {
        std::array<char, kSize + 1> result{};
        strncpy(result.data(), this->data(), kSize);

        return result;
    }

    const char *GetCStr() const noexcept { return this->data(); }
};

}  // namespace data_structures

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_DATA_STRUCTURES_ARRAY_STRUCTURES_HPP_
