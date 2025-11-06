#include <test_module/test.hpp>

// ------------------------------
// R_ASSERT_EQ Tests
// ------------------------------

TEST(RAssertEqPass)
{
    R_ASSERT_EQ(5, 5);
    R_ASSERT_EQ(true, true);
    R_ASSERT_EQ(nullptr, nullptr);

    // With formatted messages
    int expected = 5, actual = 5;
    R_ASSERT_EQ(
        expected, actual, "Expected value %d should equal actual value %d", expected, actual
    );
    bool flag = true;
    R_ASSERT_EQ(flag, flag, "Boolean flag should equal itself (value: %d)", flag);
    void *ptr = nullptr;
    R_ASSERT_EQ(nullptr, ptr, "Null pointer check for ptr at address %p", (void *)&ptr);
}

FAIL_TEST(RAssertEqFail)
{
    R_ASSERT_EQ(5, 10);

    // With formatted message
    int expected = 5, actual = 10;
    R_ASSERT_EQ(expected, actual, "Values don't match: expected=%d, actual=%d", expected, actual);
}

// ------------------------------
// R_ASSERT_NEQ Tests
// ------------------------------

TEST(RAssertNeqPass)
{
    R_ASSERT_NEQ(5, 10);
    R_ASSERT_NEQ(true, false);
    int value = 5;
    R_ASSERT_NEQ(&value, nullptr);

    // With formatted messages
    int val1 = 5, val2 = 10;
    R_ASSERT_NEQ(val1, val2, "Values should differ: %d != %d", val1, val2);
    bool flag1 = true, flag2 = false;
    R_ASSERT_NEQ(flag1, flag2, "Boolean values %d and %d should be different", flag1, flag2);
    R_ASSERT_NEQ(&value, nullptr, "Pointer %p should not be null", &value);
}

FAIL_TEST(RAssertNeqFail)
{
    R_ASSERT_NEQ(5, 5);

    // With formatted message
    int val = 5;
    R_ASSERT_NEQ(val, val, "Expected different values, but both are %d", val);
}

// ------------------------------
// R_ASSERT_TRUE Tests
// ------------------------------

TEST(RAssertTruePass)
{
    R_ASSERT_TRUE(true);
    R_ASSERT_TRUE(1 == 1);
    R_ASSERT_TRUE(5 > 3);

    // With formatted messages
    bool condition = true;
    R_ASSERT_TRUE(condition, "Condition should be true (value: %d)", condition);
    int a = 1, b = 1;
    R_ASSERT_TRUE(a == b, "Values should be equal: %d == %d", a, b);
    int x = 5, y = 3;
    R_ASSERT_TRUE(x > y, "Value %d should be greater than %d", x, y);
}

FAIL_TEST(RAssertTrueFail)
{
    R_ASSERT_TRUE(false);

    // With formatted message
    bool result = false;
    R_ASSERT_TRUE(result, "Expected true but got %d", result);
}

// ------------------------------
// R_ASSERT_FALSE Tests
// ------------------------------

TEST(RAssertFalsePass)
{
    R_ASSERT_FALSE(false);
    R_ASSERT_FALSE(1 == 2);
    R_ASSERT_FALSE(5 < 3);

    // With formatted messages
    bool condition = false;
    R_ASSERT_FALSE(condition, "Condition should be false (value: %d)", condition);
    int a = 1, b = 2;
    R_ASSERT_FALSE(a == b, "Values should not be equal: %d != %d", a, b);
    int x = 5, y = 3;
    R_ASSERT_FALSE(x < y, "Value %d should not be less than %d", x, y);
}

FAIL_TEST(RAssertFalseFail)
{
    R_ASSERT_FALSE(true);

    // With formatted message
    bool result = true;
    R_ASSERT_FALSE(result, "Expected false but got %d", result);
}

// ------------------------------
// R_ASSERT_NOT_NULL Tests
// ------------------------------

TEST(RAssertNotNullPass)
{
    int value = 5;
    int *ptr  = &value;
    R_ASSERT_NOT_NULL(ptr);

    // With formatted message
    R_ASSERT_NOT_NULL(ptr, "Pointer to int value %d should not be null", value);
}

FAIL_TEST(RAssertNotNullFail)
{
    int *ptr = nullptr;
    R_ASSERT_NOT_NULL(ptr);

    // With formatted message
    R_ASSERT_NOT_NULL(ptr, "Expected non-null pointer at address %p", (void *)&ptr);
}

// ------------------------------
// R_ASSERT_NULL Tests
// ------------------------------

TEST(RAssertNullPass)
{
    int *ptr = nullptr;
    R_ASSERT_NULL(ptr);

    // With formatted message
    R_ASSERT_NULL(ptr, "Pointer at address %p should be null", (void *)&ptr);
}

FAIL_TEST(RAssertNullFail)
{
    int value = 5;
    int *ptr  = &value;
    R_ASSERT_NULL(ptr);

    // With formatted message
    R_ASSERT_NULL(ptr, "Expected null pointer but got %p pointing to value %d", (void *)ptr, value);
}

// ------------------------------
// R_ASSERT_LT Tests
// ------------------------------

TEST(RAssertLtPass)
{
    R_ASSERT_LT(5, 10);

    // With formatted message
    int small = 5, large = 10;
    R_ASSERT_LT(small, large, "Value %d should be less than %d", small, large);
}

FAIL_TEST(RAssertLtFail)
{
    R_ASSERT_LT(10, 5);

    // With formatted message
    int large = 10, small = 5;
    R_ASSERT_LT(large, small, "Expected %d to be less than %d", large, small);
}

// ------------------------------
// R_ASSERT_LE Tests
// ------------------------------

TEST(RAssertLePass)
{
    R_ASSERT_LE(5, 5);
    R_ASSERT_LE(5, 10);

    // With formatted messages
    int a = 5, b = 5, c = 10;
    R_ASSERT_LE(a, b, "Value %d should be less than or equal to %d", a, b);
    R_ASSERT_LE(a, c, "Value %d should be less than %d", a, c);
}

FAIL_TEST(RAssertLeFail)
{
    R_ASSERT_LE(10, 5);

    // With formatted message
    int large = 10, small = 5;
    R_ASSERT_LE(large, small, "Expected %d to be less than or equal to %d", large, small);
}

// ------------------------------
// R_ASSERT_GT Tests
// ------------------------------

TEST(RAssertGtPass)
{
    R_ASSERT_GT(10, 5);

    // With formatted message
    int large = 10, small = 5;
    R_ASSERT_GT(large, small, "Value %d should be greater than %d", large, small);
}

FAIL_TEST(RAssertGtFail)
{
    R_ASSERT_GT(5, 10);

    // With formatted message
    int small = 5, large = 10;
    R_ASSERT_GT(small, large, "Expected %d to be greater than %d", small, large);
}

// ------------------------------
// R_ASSERT_GE Tests
// ------------------------------

TEST(RAssertGePass)
{
    R_ASSERT_GE(5, 5);
    R_ASSERT_GE(10, 5);

    // With formatted messages
    int a = 5, b = 5, c = 10;
    R_ASSERT_GE(a, b, "Value %d should be greater than or equal to %d", a, b);
    R_ASSERT_GE(c, a, "Value %d should be greater than %d", c, a);
}

FAIL_TEST(RAssertGeFail)
{
    R_ASSERT_GE(5, 10);

    // With formatted message
    int small = 5, large = 10;
    R_ASSERT_GE(small, large, "Expected %d to be greater than or equal to %d", small, large);
}

// ------------------------------
// R_ASSERT_STREQ Tests
// ------------------------------

TEST(RAssertStrEqPass)
{
    const char *str1 = "test";
    const char *str2 = "test";
    R_ASSERT_STREQ(str1, str2);

    // With formatted message
    R_ASSERT_STREQ(str1, str2, "Strings '%s' and '%s' should be equal", str1, str2);
}

FAIL_TEST(RAssertStrEqFail)
{
    const char *str1 = "test1";
    const char *str2 = "test2";
    R_ASSERT_STREQ(str1, str2);

    // With formatted message
    R_ASSERT_STREQ(str1, str2, "Expected '%s' to equal '%s'", str1, str2);
}

// ------------------------------
// R_ASSERT_STRNEQ Tests
// ------------------------------

TEST(RAssertStrNeqPass)
{
    const char *str1 = "test1";
    const char *str2 = "test2";
    R_ASSERT_STRNEQ(str1, str2);

    // With formatted message
    R_ASSERT_STRNEQ(str1, str2, "Strings '%s' and '%s' should be different", str1, str2);
}

FAIL_TEST(RAssertStrNeqFail)
{
    const char *str1 = "test";
    const char *str2 = "test";
    R_ASSERT_STRNEQ(str1, str2);

    // With formatted message
    R_ASSERT_STRNEQ(str1, str2, "Expected '%s' to differ from '%s'", str1, str2);
}
