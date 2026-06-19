// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_MATH_CHECKS_H_
#define LIBS_LIBC_SRC_INCLUDE_MATH_CHECKS_H_

#include <defines.h>
#include <stdint.h>

BEGIN_DECL_C

CONSTEXPR int isnan(const double num)
{
    union __DoubleBits bits;
    bits.d = num;
    return ((bits.u >> 52 & 0x7FF) == 0x7FF) && ((bits.u & (-1ULL >> 12)) != 0);
}

CONSTEXPR int isinf(const double num)
{
    union __DoubleBits bits;
    bits.d = num;
    return ((bits.u >> 52 & 0x7FF) == 0x7FF) && ((bits.u & (-1ULL >> 12)) == 0);
}

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_MATH_CHECKS_H_
