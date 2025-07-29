#ifndef LIBC_INCLUDE_ASSERT_H_
#define LIBC_INCLUDE_ASSERT_H_

#include <todo.h>

// ------------------------------
// Kernel Asserts
// ------------------------------

#ifdef __ALKOS_LIBK__

#include <panic.hpp>

#define __FAIL_KERNEL(expr)                                                                        \
    KernelPanic(                                                                                   \
        "Assertion failed: " TOSTRING(expr) " at file: " __FILE__ " and line: " TOSTRING(__LINE__) \
    );

#define __ASSERT_FAIL_FUNC KernelPanic

#define __STATIC_ASSERT_BASE(expr, msg)                                                            \
    static_assert(                                                                                 \
        expr,                                                                                      \
        "Assertion failed: " TOSTRING(expr) " at file: " __FILE__ " and line: " TOSTRING(__LINE__) \
    );

/* usual kernel assert macro */
#ifdef NDEBUG
#define ASSERT(expr)     ((void)0)
#define FAIL_ALWAYS(msg) ((void)0)
#else
#define ASSERT(expr)         \
    if (!(expr)) {           \
        __FAIL_KERNEL(expr); \
    }
#define FAIL_ALWAYS(msg) __FAIL_KERNEL(false && msg)
#endif  // NDEBUG

/* usual kernel working in release assert macro */
#define R_ASSERT(expr)       \
    if (!(expr)) {           \
        __FAIL_KERNEL(expr); \
    }
#define R_FAIL_ALWAYS(msg) __FAIL_KERNEL(false && msg)

/* libc default assert macro */
#define assert(expr) ASSERT(expr)

// ------------------------------
// Userspace asserts
// ------------------------------

#else

#define __ASSERT_FAIL_FUNC TODO_USERSPACE

#ifdef NDEBUG
#define assert(expr)     ((void)0)
#define FAIL_ALWAYS(msg) ((void)0)
#else
#define assert(expr)     TODO_USERSPACE
#define FAIL_ALWAYS(msg) TODO_USERSPACE
#endif  // NDEBUG

#endif

// ------------------------------
// C++ extended asserts
// ------------------------------

#ifdef __cplusplus

#include <defines.hpp>
#include <extensions/assert_base.hpp>

/* usual C-style asserts */
#define ASSERT_EQ(expected, value) \
    BASE_ASSERT_EQ(kIsDebugBuild, expected, value, __ASSERT_FAIL_FUNC)
#define ASSERT_NEQ(expected, value) \
    BASE_ASSERT_NEQ(kIsDebugBuild, expected, value, __ASSERT_FAIL_FUNC)
#define ASSERT_ZERO(value)        BASE_ASSERT_ZERO(kIsDebugBuild, value, __ASSERT_FAIL_FUNC)
#define ASSERT_TRUE(value)        BASE_ASSERT_TRUE(kIsDebugBuild, value, __ASSERT_FAIL_FUNC)
#define ASSERT_FALSE(value)       BASE_ASSERT_FALSE(kIsDebugBuild, value, __ASSERT_FAIL_FUNC)
#define ASSERT_NOT_NULL(value)    BASE_ASSERT_NOT_NULL(kIsDebugBuild, value, __ASSERT_FAIL_FUNC)
#define ASSERT_NULL(value)        BASE_ASSERT_NULL(kIsDebugBuild, value, __ASSERT_FAIL_FUNC)
#define ASSERT_LT(val1, val2)     BASE_ASSERT_LT(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC)
#define ASSERT_LE(val1, val2)     BASE_ASSERT_LE(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC)
#define ASSERT_GT(val1, val2)     BASE_ASSERT_GT(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC)
#define ASSERT_GE(val1, val2)     BASE_ASSERT_GE(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC)
#define ASSERT_STREQ(val1, val2)  BASE_ASSERT_STREQ(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC)
#define ASSERT_STRNEQ(val1, val2) BASE_ASSERT_STRNEQ(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC)

/* release build asserts */
#define R_ASSERT_EQ(expected, value)  BASE_ASSERT_EQ(true, expected, value, __ASSERT_FAIL_FUNC)
#define R_ASSERT_NEQ(expected, value) BASE_ASSERT_NEQ(true, expected, value, __ASSERT_FAIL_FUNC)
#define R_ASSERT_ZERO(value)          BASE_ASSERT_ZERO(true, value, __ASSERT_FAIL_FUNC)
#define R_ASSERT_TRUE(value)          BASE_ASSERT_TRUE(true, value, __ASSERT_FAIL_FUNC)
#define R_ASSERT_FALSE(value)         BASE_ASSERT_FALSE(true, value, __ASSERT_FAIL_FUNC)
#define R_ASSERT_NOT_NULL(value)      BASE_ASSERT_NOT_NULL(true, value, __ASSERT_FAIL_FUNC)
#define R_ASSERT_NULL(value)          BASE_ASSERT_NULL(true, value, __ASSERT_FAIL_FUNC)
#define R_ASSERT_LT(val1, val2)       BASE_ASSERT_LT(true, val1, val2, __ASSERT_FAIL_FUNC)
#define R_ASSERT_LE(val1, val2)       BASE_ASSERT_LE(true, val1, val2, __ASSERT_FAIL_FUNC)
#define R_ASSERT_GT(val1, val2)       BASE_ASSERT_GT(true, val1, val2, __ASSERT_FAIL_FUNC)
#define R_ASSERT_GE(val1, val2)       BASE_ASSERT_GE(true, val1, val2, __ASSERT_FAIL_FUNC)
#define R_ASSERT_STREQ(val1, val2)    BASE_ASSERT_STREQ(true, val1, val2, __ASSERT_FAIL_FUNC)
#define R_ASSERT_STRNEQ(val1, val2)   BASE_ASSERT_STRNEQ(true, val1, val2, __ASSERT_FAIL_FUNC)

/* static asserts */
#define STATIC_ASSERT(expr, msg) __STATIC_ASSERT_BASE(expr, msg)
#define STATIC_ASSERT_EQ(expected, value) \
    STATIC_ASSERT((expected == value), "Expected: " TOSTRING(expected) " Got: " TOSTRING(value))
#define STATIC_ASSERT_NEQ(expected, value) \
    STATIC_ASSERT((expected != value), "Expected: " TOSTRING(expected) Got : TOSTRING(value))
#define STATIC_ASSERT_ZERO(value) STATIC_ASSERT((value == 0), "Expected: 0 Got: " TOSTRING(value))
#define STATIC_ASSERT_TRUE(value) \
    STATIC_ASSERT((value == true), "Expected: true Got: " TOSTRING(value))
#define STATIC_ASSERT_FALSE(value) \
    STATIC_ASSERT((value == false), "Expected: false Got: " TOSTRING(value))
#define STATIC_ASSERT_NOT_NULL(value) \
    STATIC_ASSERT((value != nullptr), "Expected: not null Got: " TOSTRING(value))
#define STATIC_ASSERT_NULL(value) \
    STATIC_ASSERT((value == nullptr), "Expected: null Got: " TOSTRING(value))
#define STATIC_ASSERT_LT(val1, val2) \
    STATIC_ASSERT((val1 < val2), "Expected: " TOSTRING(val1) " < " TOSTRING(val2))
#define STATIC_ASSERT_LE(val1, val2) \
    STATIC_ASSERT((val1 <= val2), "Expected: " TOSTRING(val1) " <= " TOSTRING(val2))
#define STATIC_ASSERT_GT(val1, val2) \
    STATIC_ASSERT((val1 > val2), "Expected: " TOSTRING(val1) " > " TOSTRING(val2))
#define STATIC_ASSERT_GE(val1, val2) \
    STATIC_ASSERT((val1 >= val2), "Expected: " TOSTRING(val1) " >= " TOSTRING(val2))
#define STATIC_ASSERT_STREQ(val1, val2) \
    STATIC_ASSERT((strcmp(val1, val2) == 0), "Expected: " TOSTRING(val1) " == " TOSTRING(val2))
#define STATIC_ASSERT_STRNEQ(val1, val2) \
    STATIC_ASSERT((strcmp(val1, val2) != 0), "Expected: " TOSTRING(val1) " != " TOSTRING(val2))

#endif  // __cplusplus

#endif  // LIBC_INCLUDE_ASSERT_H_
