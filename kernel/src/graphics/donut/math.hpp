#ifndef KERNEL_SRC_GRAPHICS_DONUT_MATH_HPP_
#define KERNEL_SRC_GRAPHICS_DONUT_MATH_HPP_

#include <types.hpp>

// --------------------------------------------------------------------------------
// Minimal Math for donut Implementation
// --------------------------------------------------------------------------------

namespace Math
{
// Simple Taylor series approximation for sin/cos to avoid libm dependency
// or massive lookup tables. Precision is "good enough" for a donut.
constexpr f32 PI = 3.14159265359f;

f32 sin(f32 x)
{
    // Normalize to -PI to PI
    while (x < -PI) {
        x += 2 * PI;
    }
    while (x > PI) {
        x -= 2 * PI;
    }

    f32 res  = 0;
    f32 term = x;
    f32 k    = 1;
    for (int i = 0; i < 5; ++i) {  // 5 iterations is plenty for visual
        res += term;
        term *= -1 * x * x / ((k + 1) * (k + 2));
        k += 2;
    }
    return res;
}

f32 cos(f32 x) { return sin(x + (PI / 2.0F)); }
}  // namespace Math

#endif /* KERNEL_SRC_GRAPHICS_DONUT_MATH_HPP_ */
