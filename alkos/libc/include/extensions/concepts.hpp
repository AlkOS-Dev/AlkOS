#ifndef LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_
#define LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_

#include <extensions/type_traits.hpp>
#include "utility.hpp"

namespace std
{

template <class From, class To>
concept convertible_to = std::is_convertible_v<From, To> && requires {
    static_cast<To>(std::declval<From>());
};

}  // namespace std

#endif  // LIBC_INCLUDE_EXTENSIONS_CONCEPTS_HPP_
