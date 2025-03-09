#ifndef LIBC_INCLUDE_EXTENSIONS_ARRAY_HPP_
#define LIBC_INCLUDE_EXTENSIONS_ARRAY_HPP_

#include <assert.h>
#include <extensions/cstddef.hpp>
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

    /* iterators */
    using iterator       = value_type*;
    using const_iterator = const value_type*;

    TODO_LIBCPP_COMPLIANCE
    using reverse_iterator       = void;
    using const_reverse_iterator = void;

    // ------------------------------
    // Constructors
    // ------------------------------

    constexpr array()                             = default;
    constexpr ~array()                            = default;
    constexpr array& operator=(const array& arrr) = default;

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
        return static_cast<pointer>(mem_);
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

    // ------------------------------
    // Class fields
    // ------------------------------

    T mem_[N];
};

}  // namespace std

// ------------------------------
// std::swap
// ------------------------------

TODO_LIBCPP_COMPLIANCE

#endif  // LIBC_INCLUDE_EXTENSIONS_ARRAY_HPP_
