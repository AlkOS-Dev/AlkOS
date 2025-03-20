#include <assert.h>
extern "C" void AvxFloatingPointTest(double a, double b);

// TODO - Use test_module/test.hpp
void FloatExtensionTest()
{
    /* simple float test */
    float f1 = 0.1f;
    float f2 = 0.2f;

    f1 = f1 + f2;
    R_ASSERT_EQ(0.1f + 0.2f, f1);

    /* simple double test */
    double d1 = 0.1;
    double d2 = 0.2;

    d2 = d1 + d2;
    R_ASSERT_EQ(0.1 + 0.2, d2);

    /* simple long double test */
    long double ld1 = 0.1l;
    long double ld2 = 0.2l;

    ld2 = ld1 + ld2;
    R_ASSERT_EQ(0.1l + 0.2l, ld2);

    /* final sum */
    long double sum = f1 + d2 + ld2;
    R_ASSERT_EQ((0.1f + 0.2f) + (0.1 + 0.2) + (0.1l + 0.2l), sum);

    /* testing avx */
    AvxFloatingPointTest(d2, d1);
}
