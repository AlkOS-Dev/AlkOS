#include <extensions/initializer_list.hpp>
#include <test_module/test.hpp>
#include <trace_framework.hpp>

// ------------------------------
// Test Framework Test
// ------------------------------

/**
 * @brief Tests verifies whether Test Framework features works correctly
 */

class TestFrameworkObj : public TestGroupBase
{
    public:
    TestFrameworkObj() = default;

    ~TestFrameworkObj() override = default;

    protected:
    void Setup_() override
    {
        a = 4;
        b = 4;
        c = 4;
    }

    void TearDown_() override
    {
        R_ASSERT_EQ(4, a);
        R_ASSERT_EQ(4, b);
        R_ASSERT_EQ(4, c);
    }

    int a = 1;
    int b = 2;
    int c = 3;
};

/* should simply succeed */
TEST_F(TestFrameworkObj, TestFrameworkTestPass) {}

/* should simply fail */
FAIL_TEST_F(TestFrameworkObj, TestFrameworkTestFail) { ASSERT(false && "TestFrameworkTestFail"); }

/* verifies correct access to fields */
TEST_F(TestFrameworkObj, TestAccessMembers)
{
    const int sum = a + b + c;
    R_ASSERT_EQ(12, sum);
}

FAIL_TEST_F(TestFrameworkObj, TestAccessMembersFail)
{
    a = 5;
    b = 6;
    c = 7;
}

MTEST(SimpleManualTest) {}

// ------------------------------
// Stack Smash Test
// ------------------------------

/**
 * @brief Test should drop kernel panic due to stack smashing
 */
FAIL_TEST(StackSmashTest)
{
    static constexpr uint64_t kStackSize = 32;
    static constexpr uint64_t kWriteSize = 64;

    [[maybe_unused]] volatile char buff[kStackSize];

    for (size_t i = 0; i < kWriteSize; i++) {
        buff[i] = 'A';
    }
}

// ------------------------------
// Float operations test
// ------------------------------

extern void FloatExtensionTest();

/**
 * @brief Test should not drop kernel panic due to enabled float extensions
 *
 * @note This test is architecture dependent, simply invokes floating point instructions
 */
TEST(FloatOperationsTest) { FloatExtensionTest(); }

// ------------------------------
// Simple Exception test
// ------------------------------

/**
 *  @brief Test should simply drop 0 division exception
 */
FAIL_TEST(SimpleExceptionTest)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
    [[maybe_unused]] volatile int a = 9 / 0;
#pragma GCC diagnostic pop
}

// ------------------------------
// Preserve CPU test
// ------------------------------

/**
 * @brief Special test interrupt writes ones to all registers. Expected behavior is to not see
 * any ones in the registers after returning from interrupt.
 */
extern "C" void PreserveCpuStateTest();

TEST(PreserveCpuStateTest) { PreserveCpuStateTest(); }

MTEST(TraceFramework) {}
