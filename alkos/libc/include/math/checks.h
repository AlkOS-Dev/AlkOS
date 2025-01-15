#ifndef LIBC_INCLUDE_MATH_CHECKS_H_
#define LIBC_INCLUDE_MATH_CHECKS_H_

#include <defines.h>
#include <stdint.h>

BEGIN_DECL_C
CONSTEPXR int isnan(double num)
{
    uint64_t *intdbl = (uint64_t *)(&num);
    return ((*intdbl >> 52 & 0x7FF) == 0x7FF) && ((*intdbl & (-1ULL >> 12)) != 0);
}

CONSTEPXR int isinf(double num)
{
    uint64_t *intdbl = (uint64_t *)(&num);
    return ((*intdbl >> 52 & 0x7FF) == 0x7FF) && ((*intdbl & (-1ULL >> 12)) == 0);
}

END_DECL_C

#endif  // LIBC_INCLUDE_MATH_CHECKS_H_
