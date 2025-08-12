#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_SPAN_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_SPAN_HPP_

#include <assert.h>
#include <stdint.h>
#include <todo.h>
#include <extensions/array.hpp>
#include <extensions/initializer_list.hpp>
#include <extensions/internal/utility.hpp>
#include <extensions/memory.hpp>
#include <extensions/type_traits.hpp>

namespace std
{

inline constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

template <typename T, std::size_t Extent = dynamic_extent>
class span
{
    public:
    // ------------------------------
    // Member types
    // ------------------------------
    using element_type    = T;
    using value_type      = std::remove_cv_t<T>;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;

    TODO_LIBCPP_COMPLIANCE
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = iterator;
    using const_reverse_iterator = const_iterator;

    static constexpr size_type extent = Extent;

    private:
    static constexpr bool is_dynamic_extent_ = (Extent == std::dynamic_extent);

    template <typename U>
    static constexpr bool convertible_ = std::is_convertible_v<U (*)[], element_type (*)[]>;

    public:
    // ------------------------------
    // Constructors
    // ------------------------------

    // Empty constructor
    constexpr span() noexcept
        requires(is_dynamic_extent_)
        : data_(nullptr), size_(0)
    {
    }

    constexpr span() noexcept
        requires(!is_dynamic_extent_ && Extent == 0)
        : data_(nullptr)
    {
    }

    TODO_LIBCPP_COMPLIANCE
    // From iterator
    template <class It>
        requires(is_dynamic_extent_)
    explicit(!is_dynamic_extent_) constexpr span(It first, size_type count)
        : data_(std::to_address(first)), size_(count)
    {
    }

    template <class It>
        requires(!is_dynamic_extent_)
    explicit(!is_dynamic_extent_) constexpr span(It first, size_type count)
        : data_(std::to_address(first))
    {
        ASSERT_EQ(count, Extent);
    }

    TODO_LIBCPP_COMPLIANCE
    // From iterators range
    template <class It, class End>
        requires(!std::is_convertible_v<End, std::size_t> && is_dynamic_extent_)
    explicit(!is_dynamic_extent_) constexpr span(It first, End last)
        : data_(std::to_address(first)), size_(last - first)
    {
    }

    template <class It, class End>
        requires(!std::is_convertible_v<End, std::size_t> && !is_dynamic_extent_)
    explicit(!is_dynamic_extent_) constexpr span(It first, End last) : data_(std::to_address(first))
    {
        ASSERT_EQ(last - first, Extent);
    }

    // From C-style array
    template <std::size_t N>
        requires(is_dynamic_extent_)
    constexpr span(std::type_identity_t<element_type> (&arr)[N]) noexcept
        requires convertible_<std::remove_pointer_t<decltype(std::data(arr))>>
        : data_(arr), size_(N)
    {
    }

    template <std::size_t N>
        requires(!is_dynamic_extent_ && Extent == N)
    constexpr span(std::type_identity_t<element_type> (&arr)[N]) noexcept
        requires convertible_<std::remove_pointer_t<decltype(std::data(arr))>>
        : data_(arr)
    {
    }

    // From std::array
    template <typename U, std::size_t N>
        requires(is_dynamic_extent_)
    constexpr span(std::array<U, N>& arr) noexcept
        requires convertible_<std::remove_pointer_t<decltype(std::data(arr))>>
        : data_(arr.data()), size_(N)
    {
    }

    template <typename U, std::size_t N>
        requires(!is_dynamic_extent_ && Extent == N)
    constexpr span(std::array<U, N>& arr) noexcept
        requires convertible_<std::remove_pointer_t<decltype(std::data(arr))>>
        : data_(arr.data())
    {
    }

    template <typename U, std::size_t N>
        requires(is_dynamic_extent_)
    constexpr span(const std::array<U, N>& arr) noexcept
        requires convertible_<std::remove_pointer_t<decltype(std::data(arr))>>
        : data_(arr.data()), size_(N)
    {
    }

    template <typename U, std::size_t N>
        requires(!is_dynamic_extent_ && Extent == N)
    constexpr span(const std::array<U, N>& arr) noexcept
        requires convertible_<std::remove_pointer_t<decltype(std::data(arr))>>
        : data_(arr.data())
    {
    }

    // From std::initializer_list
    explicit(extent != std::dynamic_extent) constexpr span(
        std::initializer_list<value_type> il
    ) noexcept
        requires std::is_const_v<element_type> && (is_dynamic_extent_)
        : data_(il.begin()), size_(il.size())
    {
    }

    explicit(extent != std::dynamic_extent) constexpr span(
        std::initializer_list<value_type> il
    ) noexcept
        requires std::is_const_v<element_type> && (!is_dynamic_extent_)
        : data_(il.begin())
    {
        ASSERT_EQ(il.size(), Extent);
    }

    // Conversion from std::span
    template <class U, std::size_t N>
        requires convertible_<U> && (is_dynamic_extent_)
    explicit(!is_dynamic_extent_ && N == std::dynamic_extent) constexpr span(
        const std::span<U, N>& source
    ) noexcept
        : data_(source.data()), size_(source.size())
    {
    }

    template <class U, std::size_t N>
        requires convertible_<U> && (!is_dynamic_extent_ && N == std::dynamic_extent)
    explicit(!is_dynamic_extent_ && N == std::dynamic_extent) constexpr span(
        const std::span<U, N>& source
    ) noexcept
        : data_(source.data()), size_(source.size())
    {
        ASSERT_EQ(source.size(), Extent);
    }

    template <class U, std::size_t N>
        requires convertible_<U> && (!is_dynamic_extent_ && N != std::dynamic_extent && Extent == N)
    explicit(!is_dynamic_extent_ && N == std::dynamic_extent) constexpr span(
        const std::span<U, N>& source
    ) noexcept
        : data_(source.data())
    {
        ASSERT_EQ(source.size(), Extent);
    }

    constexpr span(const span& other) noexcept = default;

    constexpr span& operator=(const span& other) noexcept = default;

    // ------------------------------
    // Iterators
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr iterator begin() noexcept { return data_; }

    NODISCARD FORCE_INLINE_F constexpr const_iterator begin() const noexcept { return data_; }

    NODISCARD FORCE_INLINE_F constexpr const_iterator cbegin() const noexcept { return data_; }

    NODISCARD FORCE_INLINE_F constexpr iterator end() noexcept { return data_ + size(); }

    NODISCARD FORCE_INLINE_F constexpr const_iterator end() const noexcept
    {
        return data_ + size();
    }

    NODISCARD FORCE_INLINE_F constexpr const_iterator cend() const noexcept
    {
        return data_ + size();
    }

    /* Reverse iterators */

    NODISCARD FORCE_INLINE_F constexpr reverse_iterator rbegin() noexcept { return end(); }

    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator rbegin() const noexcept
    {
        return end();
    }

    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator crbegin() const noexcept
    {
        return end();
    }

    NODISCARD FORCE_INLINE_F constexpr reverse_iterator rend() noexcept { return begin(); }

    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator rend() const noexcept
    {
        return begin();
    }

    NODISCARD FORCE_INLINE_F constexpr const_reverse_iterator crend() const noexcept
    {
        return begin();
    }

    // ------------------------------
    // Element access
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr reference front() const
    {
        ASSERT(!empty());
        return data_[0];
    }

    NODISCARD FORCE_INLINE_F constexpr reference back() const
    {
        ASSERT(!empty());
        return data_[size() - 1];
    }

    NODISCARD FORCE_INLINE_F constexpr reference at(size_type pos) const
    {
        R_ASSERT_LT(pos, size());
        return data_[pos];
    }

    NODISCARD FORCE_INLINE_F constexpr reference operator[](size_type idx) const
    {
        if constexpr (is_dynamic_extent_) {
            ASSERT_LT(idx, size());
        } else {
            ASSERT_LT(idx, Extent);
        }
        return data_[idx];
    }

    NODISCARD FORCE_INLINE_F constexpr pointer data() const noexcept { return data_; }

    // ------------------------------
    // Observers
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_type size() const noexcept
    {
        if constexpr (is_dynamic_extent_) {
            return size_;
        } else {
            return Extent;
        }
    }

    NODISCARD FORCE_INLINE_F constexpr size_type size_bytes() const noexcept
    {
        return size() * sizeof(element_type);
    }

    NODISCARD FORCE_INLINE_F constexpr bool empty() const noexcept { return size() == 0; }

    // ------------------------------
    // Subspan
    // ------------------------------

    template <std::size_t Count>
        requires(Count <= Extent)
    NODISCARD FORCE_INLINE_F constexpr std::span<element_type, Count> first() const
    {
        ASSERT_LE(Count, size());
        return std::span<element_type, Count>(data_, Count);
    }

    NODISCARD FORCE_INLINE_F constexpr std::span<element_type> first(size_type count) const
    {
        ASSERT_LE(count, size());
        return std::span<element_type>(data_, count);
    }

    template <std::size_t Count>
        requires(Count <= Extent)
    NODISCARD FORCE_INLINE_F constexpr std::span<element_type, Count> last() const
    {
        ASSERT_LE(Count, size());
        return std::span<element_type, Count>(data() + (size() - Count), Count);
    }

    NODISCARD FORCE_INLINE_F constexpr std::span<element_type> last(size_type count) const
    {
        ASSERT_LE(count, size());
        return std::span<element_type>(data() + (size() - count), count);
    }

    template <std::size_t Offset, std::size_t Count = std::dynamic_extent>
        requires(Offset > Extent || (Count != std::dynamic_extent && Count + Offset > Extent))
    NODISCARD FORCE_INLINE_F constexpr auto subspan() const
    {
        constexpr bool is_dynamic_extent = (Count == std::dynamic_extent);
        ASSERT(Offset <= size() && (is_dynamic_extent || Count <= size() - Offset));

        constexpr size_t count =
            !is_dynamic_extent ? Count
                               : (!is_dynamic_extent_ ? Extent - Offset : std::dynamic_extent);
        return std::span<element_type, count>(
            data() + Offset, !is_dynamic_extent ? Count : size() - Offset
        );
    }

    constexpr std::span<element_type> subspan(
        size_type offset, size_type count = std::dynamic_extent
    ) const
    {
        const bool is_dynamic_extent = (count == std::dynamic_extent);
        ASSERT(offset <= size() && (is_dynamic_extent || count <= size() - offset));

        return std::span<element_type>(
            data() + offset, !is_dynamic_extent ? count : size() - offset
        );
    }

    private:
    // ------------------------------
    // Data members
    // ------------------------------
    NO_UNIQUE_ADDRESS pointer data_;
    NO_UNIQUE_ADDRESS std::conditional_t<is_dynamic_extent_, size_type, UNIQUE_EMPTY> size_;
};

// ------------------------------
// std::as_bytes
// ------------------------------

template <class T, std::size_t N>
FORCE_INLINE_F constexpr auto as_bytes(std::span<T, N> s) noexcept
{
    constexpr size_t count = N == std::dynamic_extent ? std::dynamic_extent : N * sizeof(T);
    return std::span<const byte, count>(reinterpret_cast<const byte*>(s.data()), s.size_bytes());
}

// ------------------------------
// std::as_writable_bytes
// ------------------------------

template <class T, std::size_t N>
    requires(!std::is_const_v<T>)
FORCE_INLINE_F constexpr auto as_writable_bytes(std::span<T, N> s) noexcept
{
    constexpr size_t count = N == std::dynamic_extent ? std::dynamic_extent : N * sizeof(T);
    return std::span<byte, count>(reinterpret_cast<byte*>(s.data()), s.size_bytes());
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_SPAN_HPP_
