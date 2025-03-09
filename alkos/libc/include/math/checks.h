#ifndef LIBC_INCLUDE_MATH_CHECKS_H_
#define LIBC_INCLUDE_MATH_CHECKS_H_

#include <defines.h>
#include <stdint.h>

BEGIN_DECL_C

CONSTEXPR int isnan(const double num)
{
    const __DoubleBits bits{.d = num};
    return ((bits.u >> 52 & 0x7FF) == 0x7FF) && ((bits.u & (-1ULL >> 12)) != 0);
}

CONSTEXPR int isinf(const double num)
{
    const __DoubleBits bits{.d = num};
    return ((bits.u >> 52 & 0x7FF) == 0x7FF) && ((bits.u & (-1ULL >> 12)) == 0);
}

END_DECL_C

#endif  // LIBC_INCLUDE_MATH_CHECKS_H_
