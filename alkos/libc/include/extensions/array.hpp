#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_ARRAY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_ARRAY_HPP_

#include <assert.h>
#include <memory.h>
#include <extensions/cstddef.hpp>
#include <extensions/initializer_list.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/utility.hpp>

namespace std
{
template <class T, std::size_t N>
    requires std::is_move_constructible_v<T> && std::is_move_assignable_v<T>
struct array {
    // ------------------------------
    // Member types
    // ------------------------------

    /* type usings */
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = T*;
    using const_pointer   = const T*;

    // TODO: replace with real ones
    /* iterators */
    using iterator       = value_type*;
    using const_iterator = const value_type*;

    TODO_LIBCPP_COMPLIANCE
    using reverse_iterator       = iterator;
    using const_reverse_iterator = const_iterator;

    // ------------------------------
    // Capacity
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr size_type size() const noexcept { return N; }

    NODISCARD FORCE_INLINE_F constexpr size_type max_size() const noexcept { return N; }

    NODISCARD FORCE_INLINE_F constexpr size_type empty() const noexcept { return size() == 0; }

    // ------------------------------
    // Element access
    // ------------------------------

    NODISCARD FORCE_INLINE_F constexpr reference operator[](const size_type n) noexcept
    {
        if constexpr (kIsKernel) {
            assert(n < size());
        }

        return mem_[n];
    }

    NODISCARD FORCE_INLINE_F constexpr const_reference operator[](const size_type n) const noexcept
    {
        if constexpr (kIsKernel) {
            assert(n < size());
        }

        return mem_[n];
    }

    NODISCARD FORCE_INLINE_F constexpr reference at(const size_type n)
    {
        if constexpr (kIsKernel) {
            R_ASSERT(n < size());
        } else {
            TODO_USERSPACE
        }

        return mem_[n];
    }

    NODISCARD FORCE_INLINE_F constexpr const_reference at(const size_type n) const
    {
        if constexpr (kIsKernel) {
            R_ASSERT(n < size());
        } else {
            TODO_USERSPACE
        }

        return mem_[n];
    }

    NODISCARD FORCE_INLINE_F constexpr reference front() noexcept
    {
        if constexpr (kIsKernel) {
            assert(!empty());
        }

        return mem_[0_size];
    }

    NODISCARD FORCE_INLINE_F constexpr const_reference front() const noexcept
    {
        if constexpr (kIsKernel) {
            assert(!empty());
        }

        return mem_[0_size];
    }

    NODISCARD FORCE_INLINE_F constexpr reference back() noexcept
    {
        if constexpr (kIsKernel) {
            assert(!empty());
        }

        return mem_[size() - 1];
    }

    NODISCARD FORCE_INLINE_F constexpr const_reference back() const noexcept
    {
        if constexpr (kIsKernel) {
            assert(!empty());
        }

        return mem_[size() - 1];
    }

    NODISCARD FORCE_INLINE_F constexpr pointer data() noexcept
    {
        return static_cast<pointer>(mem_);
    }

    NODISCARD FORCE_INLINE_F constexpr const_pointer data() const noexcept
    {
        return static_cast<const_pointer>(mem_);
    }

    // ------------------------------
    // Operations
    // ------------------------------

    constexpr void fill(const_reference value)
    {
        for (auto& ref : mem_) {
            ref = value;
        }
    }

    constexpr void swap(array& other) TODO_LIBCPP_COMPLIANCE
    // noexcept(is_nothrow_swappable)
    {
        std::swap(mem_, other.mem_);
    }

    // ------------------------------
    // Iterators
    // ------------------------------

    NODISCARD FORCE_INLINE_F iterator begin() noexcept { return mem_; }

    NODISCARD FORCE_INLINE_F const_iterator begin() const noexcept { return mem_; }

    NODISCARD FORCE_INLINE_F const_iterator cbegin() const noexcept { return mem_; }

    NODISCARD FORCE_INLINE_F iterator end() noexcept { return mem_ + size(); }

    NODISCARD FORCE_INLINE_F const_iterator end() const noexcept { return mem_ + size(); }

    NODISCARD FORCE_INLINE_F const_iterator cend() const noexcept { return mem_ + size(); }

    /* Reverse iterators */

    NODISCARD FORCE_INLINE_F reverse_iterator rbegin() noexcept { return end(); }

    NODISCARD FORCE_INLINE_F const_reverse_iterator rbegin() const noexcept { return end(); }

    NODISCARD FORCE_INLINE_F const_reverse_iterator crbegin() const noexcept { return end(); }

    NODISCARD FORCE_INLINE_F reverse_iterator rend() noexcept { return begin(); }

    NODISCARD FORCE_INLINE_F const_reverse_iterator rend() const noexcept { return begin(); }

    NODISCARD FORCE_INLINE_F const_reverse_iterator crend() const noexcept { return begin(); }

    // ------------------------------
    // Class fields
    // ------------------------------

    T mem_[N];
};

template <class U, std::size_t M>
NODISCARD constexpr bool operator==(const array<U, M>& a, const array<U, M>& b) noexcept
{
    TODO_OPTIMISE
    /* TODO: here based if the type is easy comparable we can use memcmp otherwise not */

    for (size_t i = 0; i < M; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

// ------------------------------
// std::swap
// ------------------------------

template <class U, std::size_t N>
static void constexpr swap(array<U, N>& a, array<U, N>& b) TODO_LIBCPP_COMPLIANCE
    // TODO: noexcept(is_nothrow_swappable_v<U>)
    noexcept
{
    a.swap(b);
}

// ------------------------------
// std::get
// ------------------------------

template <std::size_t I, class T, std::size_t N>
NODISCARD FORCE_INLINE_F constexpr T& get(array<T, N>& arr) noexcept
{
    static_assert(I < N, "Index out of bounds in std::get<>");
    return arr[I];
}

template <std::size_t I, class T, std::size_t N>
NODISCARD FORCE_INLINE_F constexpr const T& get(const array<T, N>& arr) noexcept
{
    static_assert(I < N, "Index out of bounds in std::get<>");
    return arr[I];
}

template <std::size_t I, class T, std::size_t N>
NODISCARD FORCE_INLINE_F constexpr T&& get(array<T, N>&& arr) noexcept
{
    static_assert(I < N, "Index out of bounds in std::get<>");
    return std::move(arr[I]);
}

template <std::size_t I, class T, std::size_t N>
NODISCARD FORCE_INLINE_F constexpr const T&& get(const array<T, N>&& arr) noexcept
{
    static_assert(I < N, "Index out of bounds in std::get<>");
    return std::move(arr[I]);
}

}  // namespace std

TODO_LIBCPP_COMPLIANCE

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_ARRAY_HPP_
