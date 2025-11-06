#ifndef ALKOS_LIBC_INCLUDE_MATH_OPERATIONS_HPP_
#define ALKOS_LIBC_INCLUDE_MATH_OPERATIONS_HPP_

#include <defines.h>
#include <stdint.h>

BEGIN_DECL_C
double modf(double num, double *integralPart);

CONSTEXPR double fabs(const double num)
{
    __DoubleBits bits{.d = num};
    bits.u &= ~(1ULL << 63);
    return bits.d;
}

END_DECL_C

#endif  // ALKOS_LIBC_INCLUDE_MATH_OPERATIONS_HPP_
