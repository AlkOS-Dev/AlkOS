#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_

#include <type_traits.hpp>
#include "utility.hpp"

namespace std
{

template <class From, class To>
concept convertible_to =
    std::is_convertible_v<From, To> && requires { static_cast<To>(std::declval<From>()); };

template <class T, class U>
concept same_as = std::is_same_v<T, U> && std::is_same_v<U, T>;

template <class T>
concept integral = is_integral_v<T>;

template <class T>
concept signed_integral = integral<T> && is_signed_v<T>;

template <class T>
concept unsigned_integral = integral<T> && !signed_integral<T>;

template <class T>
concept floating_point = is_floating_point_v<T>;

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_
