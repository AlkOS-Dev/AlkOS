#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_

#include <extensions/compare.hpp>
#include <extensions/defines.hpp>
#include <extensions/type_traits.hpp>

namespace std
{
// ------------------------------
// std::forward
// ------------------------------

template <typename T>
NODISCARD FORCE_INLINE_F constexpr T &&forward(remove_reference_t<T> &t) noexcept
{
    return static_cast<T &&>(t);
}

template <typename T>
NODISCARD FORCE_INLINE_F constexpr T &&forward(remove_reference_t<T> &&t) noexcept
{
    static_assert(
        !is_lvalue_reference_v<T>, "std::forward must not be used to convert an rvalue to an lvalue"
    );
    return static_cast<T &&>(t);
}

// ------------------------------
// std::move
// ------------------------------

template <typename T>
NODISCARD FORCE_INLINE_F constexpr remove_reference_t<T> &&move(T &&t) noexcept
{
    return static_cast<remove_reference_t<T> &&>(t);
}

// ------------------------------
// std::declval
// ------------------------------

namespace internal
{
template <class T, class U = T &&>
U declval(int) noexcept;

template <class T>
T declval(...) noexcept;
}  // namespace internal

template <typename T>
auto declval() noexcept -> decltype(internal::declval<T>(0))
{
    static_assert(false, "declval should not be used");
    return internal::declval<T>();
}

// ------------------------------
// std::swap
// ------------------------------

template <class T>
    requires std::is_move_constructible_v<T> && std::is_move_assignable_v<T>
constexpr void swap(
    T &a, T &b
) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
{
    T tmp = std::move(a);
    a     = std::move(b);
    b     = std::move(tmp);
}

namespace internal
{
template <class T, size_t N>
using arr_t = T[N];
}

TODO_LIBCPP_COMPLIANCE
// requires std::is_swapable_v<T>
//  noexcept(...)
template <class T, size_t N>
constexpr void swap(internal::arr_t<T, N> &a, internal::arr_t<T, N> &b)
{
    for (size_t idx = 0; idx < N; ++idx) {
        std::swap(a[idx], b[idx]);
    }
}

// ------------------------------
// std::in_place_t
// ------------------------------

// For tag dispatching to in-place construct an object.
struct in_place_t {
    explicit in_place_t() = default;
};

inline constexpr in_place_t in_place{};

// For tag dispatching to in-place construct an object of a specific type.
template <class T>
struct in_place_type_t {
    explicit in_place_type_t() = default;
};

template <class T>
inline constexpr in_place_type_t<T> in_place_type{};

// For tag dispatching to in-place construct an object at a specific index.
template <size_t I>
struct in_place_index_t {
    explicit in_place_index_t() = default;
};

template <size_t I>
inline constexpr in_place_index_t<I> in_place_index{};

}  // namespace std
#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_
