#include <math.h>
#include <stdint.h>

double modf(double num, double *integralPart)
{
    auto intdbl         = reinterpret_cast<uint64_t *>(&num);
    const auto exponent = static_cast<short>((*intdbl >> 52 & 0x7FF) - 1023);

    // No integral part
    if (exponent < 0) {
        *integralPart = 0.0;
        return num;
    }

    // No fractional part (Over 2^52 the representable numbers are exactly the integers)
    if (exponent >= 52) {
        *integralPart = num;
        return 0.0;
    }

    // Mantissa mask
    const uint64_t mask = -1ULL >> 12 >> exponent;

    // No fractional part
    if ((*intdbl & mask) == 0) {
        *integralPart = num;
        return 0.0;
    }

    uint64_t tmp  = *intdbl & ~mask;
    *integralPart = *reinterpret_cast<double *>(&tmp);

    return num - *integralPart;
}
