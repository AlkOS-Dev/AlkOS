// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_VARIANT_HPP_
#define LIBS_LIBCPP_INCLUDE_VARIANT_HPP_

namespace std
{
// ------------------------------
// std::monostate
// ------------------------------

struct monostate {
};
constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr strong_ordering operator<=>(monostate, monostate) noexcept
{
    return strong_ordering::equal;
}
}  // namespace std

#endif  // LIBS_LIBCPP_INCLUDE_VARIANT_HPP_
