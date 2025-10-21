#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_UTILITY_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_UTILITY_HPP_

#include <extensions/initializer_list.hpp>

namespace std
{
// ------------------------------
// std::data
// ------------------------------

template <class C>
constexpr auto data(C &c) -> decltype(c.data())
{
    return c.data();
}

template <class C>
constexpr auto data(const C &c) -> decltype(c.data())
{
    return c.data();
}

template <class T, std::size_t N>
constexpr T *data(T (&array)[N]) noexcept
{
    return array;
}

template <class E>
constexpr const E *data(std::initializer_list<E> il) noexcept
{
    return il.begin();
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_UTILITY_HPP_
