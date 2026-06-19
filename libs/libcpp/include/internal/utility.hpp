// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_INTERNAL_UTILITY_HPP_
#define LIBS_LIBCPP_INCLUDE_INTERNAL_UTILITY_HPP_

#include <initializer_list.hpp>

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

#endif  // LIBS_LIBCPP_INCLUDE_INTERNAL_UTILITY_HPP_
