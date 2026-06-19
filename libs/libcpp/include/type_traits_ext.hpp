// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_TYPE_TRAITS_EXT_HPP_
#define LIBS_LIBCPP_INCLUDE_TYPE_TRAITS_EXT_HPP_

#include <type_traits.hpp>

namespace type_traits_ext
{

template <bool B, typename T>
struct conditional_const : std::conditional<B, std::add_const_t<T>, std::remove_const_t<T>> {
};

template <bool B, typename T>
using conditional_const_t = typename conditional_const<B, T>::type;

}  // namespace type_traits_ext

#endif  // LIBS_LIBCPP_INCLUDE_TYPE_TRAITS_EXT_HPP_
