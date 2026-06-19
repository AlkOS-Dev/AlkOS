// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_INTERNAL_EMPTY_HPP_
#define LIBS_LIBCPP_INCLUDE_INTERNAL_EMPTY_HPP_

#include <compare.hpp>

// ------------------------------
// Unique empty struct
// ------------------------------
template <size_t ID>
struct empty_t {
};

#define UNIQUE_EMPTY empty_t<__COUNTER__>

template <size_t ID, size_t OtherID>
constexpr bool operator==(empty_t<ID>, empty_t<OtherID>) noexcept
{
    return true;
}

template <size_t ID, size_t OtherID>
constexpr std::strong_ordering operator<=>(empty_t<ID>, empty_t<OtherID>) noexcept
{
    return std::strong_ordering::equal;
}

#endif  // LIBS_LIBCPP_INCLUDE_INTERNAL_EMPTY_HPP_
