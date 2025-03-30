#ifndef LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_
#define LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_

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

/* declval */
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

//------------------------------------------------------------------------------//
// std::in_place
//------------------------------------------------------------------------------//

struct in_place_t {
    explicit in_place_t() = default;
};
inline constexpr in_place_t in_place{};

template <typename T>
struct in_place_type_t {
    explicit in_place_type_t() = default;
};
template <typename T>
inline constexpr in_place_type_t<T> in_place_type{};

template <size_t I>
struct in_place_index_t {
    explicit in_place_index_t() = default;
};
template <size_t I>
inline constexpr in_place_index_t<I> in_place_index{};

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

template <class T, size_t N>
TODO_LIBCPP_COMPLIANCE
    // requires std::is_swapable_v<T>
    //  noexcept(...)
    constexpr void
    swap(internal::arr_t<T, N> &a, internal::arr_t<T, N> &b)
{
    for (size_t idx = 0; idx < N; ++idx) {
        std::swap(a[idx], b[idx]);
    }
}

}  // namespace std
#endif  // LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_
