#include <math.h>
#include <test_module/test.hpp>

class MathTest : public TestGroupBase
{
    protected:
    static constexpr double kEpsilon = 1e-10;

    // Helper to check if two doubles are approximately equal
    static bool approxEqual(const double a, const double b) { return fabs(a - b) < kEpsilon; }
};

// ------------------------------
// fabs tests
// ------------------------------

TEST_F(MathTest, Fabs)
{
    R_ASSERT_TRUE(approxEqual(fabs(3.14159), 3.14159));
    R_ASSERT_TRUE(approxEqual(fabs(-3.14159), 3.14159));
    R_ASSERT_TRUE(approxEqual(fabs(0.0), 0.0));
    R_ASSERT_TRUE(approxEqual(fabs(-0.0), 0.0));
}

// ------------------------------
// modf tests
// ------------------------------

TEST_F(MathTest, Modf)
{
    double intPart;
    double fracPart;

    /* positive number */
    fracPart = modf(3.14159, &intPart);
    R_ASSERT_TRUE(approxEqual(intPart, 3.0));
    R_ASSERT_TRUE(approxEqual(fracPart, 0.14159));

    /* negative number */
    fracPart = modf(-3.14159, &intPart);
    R_ASSERT_TRUE(approxEqual(intPart, -3.0));
    R_ASSERT_TRUE(approxEqual(fracPart, -0.14159));

    /* Whole number */
    fracPart = modf(42.0, &intPart);
    R_ASSERT_TRUE(approxEqual(intPart, 42.0));
    R_ASSERT_TRUE(approxEqual(fracPart, 0.0));

    /* zero */
    fracPart = modf(0.0, &intPart);
    R_ASSERT_TRUE(approxEqual(intPart, 0.0));
    R_ASSERT_TRUE(approxEqual(fracPart, 0.0));
}
