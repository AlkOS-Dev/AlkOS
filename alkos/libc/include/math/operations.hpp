#ifndef LIBC_INCLUDE_MATH_OPERATIONS_H_
#define LIBC_INCLUDE_MATH_OPERATIONS_H_

#include <defines.h>
#include <stdint.h>

BEGIN_DECL_C
double modf(double num, double *integralPart);

CONSTEPXR double fabs(double num)
{
    uint64_t *intdbl = (uint64_t *)(&num);
    *intdbl &= ~(1ULL << 63);
    return num;
}

END_DECL_C

#endif  // LIBC_INCLUDE_MATH_OPERATIONS_H_
