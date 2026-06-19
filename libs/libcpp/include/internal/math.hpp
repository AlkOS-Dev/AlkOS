// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_INTERNAL_MATH_HPP_
#define LIBS_LIBCPP_INCLUDE_INTERNAL_MATH_HPP_

#include <defines.hpp>
#include <type_traits.hpp>

namespace internal
{
template <typename Dividend, typename Divisor>
NODISCARD FORCE_INLINE_F constexpr auto DivRoundUp(Dividend x, Divisor y)
{
    if constexpr (std::is_unsigned_v<Dividend> && std::is_unsigned_v<Divisor>) {
        return (x + y - 1) / y;  // uint / uint
    } else if constexpr (std::is_signed_v<Dividend> && std::is_unsigned_v<Divisor>) {
        auto sy = static_cast<std::make_signed_t<Divisor>>(y);
        return x / sy + (x % sy != 0 && x >= 0);  // int / uint
    } else if constexpr (std::is_unsigned_v<Dividend> && std::is_signed_v<Divisor>) {
        auto sx = static_cast<std::make_signed_t<Dividend>>(x);
        return sx / y + (sx % y != 0 && y >= 0);  // uint / int
    } else {
        return x / y + (x % y != 0 && (y >= 0) == (x >= 0));  // int / int
    }
}

}  // namespace internal

#endif  // LIBS_LIBCPP_INCLUDE_INTERNAL_MATH_HPP_
