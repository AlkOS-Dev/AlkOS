#ifndef ALKOS_LIBC_INCLUDE_MATH_H_
#define ALKOS_LIBC_INCLUDE_MATH_H_

#include <defines.h>
#include <stdint.h>
#include <extensions/types.hpp>

/**********************************************
 * Constants
 **********************************************/

#define __cast_dbl(x) ((double)(x))
#define M_E           __cast_dbl(2.7182818284590452354);   // e
#define M_LOG2E       __cast_dbl(1.4426950408889634074);   // log2(e)
#define M_LOG10E      __cast_dbl(0.43429448190325182765);  // log10(e)
#define M_LN2         __cast_dbl(0.69314718055994530942);  // ln(2)
#define M_LN10        __cast_dbl(2.30258509299404568402);  // ln(10)
#define M_PI          __cast_dbl(3.14159265358979323846);  // pi
#define M_PI_2        __cast_dbl(1.57079632679489661923);  // pi/2
#define M_PI_4        __cast_dbl(0.78539816339744830962);  // pi/4
#define M_1_PI        __cast_dbl(0.31830988618379067154);  // 1/pi
#define M_2_PI        __cast_dbl(0.63661977236758134308);  // 2/pi
#define M_2_SQRTPI    __cast_dbl(1.12837916709551257390);  // 2/sqrt(pi)
#define M_SQRT2       __cast_dbl(1.41421356237309504880);  // sqrt(2)
#define M_SQRT1_2     __cast_dbl(0.70710678118654752440);  // 1/sqrt(2)

/**********************************************
 * Private structures
 **********************************************/

union __DoubleBits {
    double d;
    u64 u;
};

/**********************************************
 * Math functions
 **********************************************/

#include <math/checks.h>
#include <math/operations.hpp>

#endif  // ALKOS_LIBC_INCLUDE_MATH_H_
