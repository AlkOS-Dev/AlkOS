#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_

#include <extensions/type_traits.hpp>
#include "utility.hpp"

namespace std
{

template <class From, class To>
concept convertible_to =
    std::is_convertible_v<From, To> && requires { static_cast<To>(std::declval<From>()); };

template <class T, class U>
concept same_as = std::is_same_v<T, U> && std::is_same_v<U, T>;

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_
