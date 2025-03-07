#ifndef LIBC_INCLUDE_MATH_OPERATIONS_H_
#define LIBC_INCLUDE_MATH_OPERATIONS_H_

#include <defines.h>
#include <stdint.h>

BEGIN_DECL_C
double modf(double num, double *integralPart);

CONSTEPXR double fabs(const double num)
{
    __DoubleBits bits{.d = num};
    bits.u &= ~(1ULL << 63);
    return bits.d;
}

END_DECL_C

#endif  // LIBC_INCLUDE_MATH_OPERATIONS_H_
