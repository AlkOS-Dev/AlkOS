#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_

#include <extensions/defines.hpp>
#include <extensions/type_traits.hpp>

namespace std
{
/* STL forward */
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

/* STL move */
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
}  // namespace std
#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_UTILITY_HPP_
