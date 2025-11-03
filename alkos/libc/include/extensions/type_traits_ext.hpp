#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_TYPE_TRAITS_EXT_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_TYPE_TRAITS_EXT_HPP_

#include <extensions/type_traits.hpp>

namespace type_traits_ext
{

template <bool B, typename T>
struct conditional_const : std::conditional<B, std::add_const_t<T>, std::remove_const_t<T>> {
};

template <bool B, typename T>
using conditional_const_t = typename conditional_const<B, T>::type;

}  // namespace type_traits_ext

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_TYPE_TRAITS_EXT_HPP_
