#ifndef ALKOS_KERNEL_TEST_TEST_MODULE_EXPECT_HPP_
#define ALKOS_KERNEL_TEST_TEST_MODULE_EXPECT_HPP_

#include <assert.h>
#include <terminal.hpp>
#include <test_module/test_module.hpp>

inline void ExpectHandler(const char* msg)
{
    test::g_testCheckFailed = true;
    TerminalWriteString(msg);
}

// ------------------------------
// Expect definitions
// ------------------------------

#define EXPECT_EQ(expected, value)  BASE_ASSERT_EQ(true, expected, value, ExpectHandler)
#define EXPECT_NEQ(expected, value) BASE_ASSERT_NEQ(true, expected, value, ExpectHandler)
#define EXPECT_ZERO(value)          BASE_ASSERT_ZERO(true, value, ExpectHandler)
#define EXPECT_TRUE(value)          BASE_ASSERT_TRUE(true, value, ExpectHandler)
#define EXPECT_FALSE(value)         BASE_ASSERT_FALSE(true, value, ExpectHandler)
#define EXPECT_NOT_NULL(value)      BASE_ASSERT_NOT_NULL(true, value, ExpectHandler)
#define EXPECT_NULL(value)          BASE_ASSERT_NULL(true, value, ExpectHandler)
#define EXPECT_LT(val1, val2)       BASE_ASSERT_LT(true, val1, val2, ExpectHandler)
#define EXPECT_LE(val1, val2)       BASE_ASSERT_LE(true, val1, val2, ExpectHandler)
#define EXPECT_GT(val1, val2)       BASE_ASSERT_GT(true, val1, val2, ExpectHandler)
#define EXPECT_GE(val1, val2)       BASE_ASSERT_GE(true, val1, val2, ExpectHandler)
#define EXPECT_STREQ(val1, val2)    BASE_ASSERT_STREQ(true, val1, val2, ExpectHandler)
#define EXPECT_STRNEQ(val1, val2)   BASE_ASSERT_STRNEQ(true, val1, val2, ExpectHandler)

#endif  // ALKOS_KERNEL_TEST_TEST_MODULE_EXPECT_HPP_
