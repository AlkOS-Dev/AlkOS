#include <test_module/test.hpp>

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

// ---------------------------------------
// cpp compatibility test components
// ---------------------------------------

class TestClass
{
    public:
    explicit TestClass(const int x) : m_a(x), m_b(2 * x + 2) {}

    int m_a;
    const int m_b;
};

TestClass g_GlobalTestClass(5);

/**
 * @brief no exception should be thrown and global object should be correctly initialized
 */
TEST(CppTest)
{
    char mem[sizeof(TestClass)];

    /* Test static initializer */
    static TestClass test(5);
    test.m_a++;

    /* test usual new and delete */
    const auto *test2 = new int;
    delete test2;

    /* test array new and delete */
    const auto *test3 = new int[5];
    delete[] test3;

    /* test placement new and delete */
    auto *test4 = new (reinterpret_cast<TestClass *>(mem)) TestClass(5);
    test4->m_a++;

    /* test global object */
    R_ASSERT_EQ(12, g_GlobalTestClass.m_b);
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
