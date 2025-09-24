#ifndef ALKOS_LIBC_INCLUDE_ASSERT_H_
#define ALKOS_LIBC_INCLUDE_ASSERT_H_

#include <todo.h>
#include "platform.h"

// ------------------------------
// Kernel Asserts
// ------------------------------

#define __FAIL(expr)                                                                               \
    __platform_panic(                                                                              \
        "Assertion failed: " TOSTRING(expr) " at file: " __FILE__ " and line: " TOSTRING(__LINE__) \
    );

#define __ASSERT_FAIL_FUNC __platform_panic

#ifdef NDEBUG
#define ASSERT(expr)     ((void)0)
#define FAIL_ALWAYS(msg) ((void)0)
#else
#define ASSERT(expr)            \
    if (!(expr)) [[unlikely]] { \
        __FAIL(expr);           \
    }
#define FAIL_ALWAYS(msg) __FAIL(false && msg)
#endif  // NDEBUG

#define R_ASSERT(expr)          \
    if (!(expr)) [[unlikely]] { \
        __FAIL(expr);           \
    }
#define R_FAIL_ALWAYS(msg) __FAIL(false && msg)

#define assert(expr) ASSERT(expr)

// ------------------------------
// C++ extended asserts
// ------------------------------

#ifdef __cplusplus

#include <extensions/assert_base.hpp>
#include <extensions/defines.hpp>

/* usual C-style asserts */
#define ASSERT_EQ(expected, value, ...) \
    BASE_ASSERT_EQ(kIsDebugBuild, expected, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_NEQ(expected, value, ...) \
    BASE_ASSERT_NEQ(kIsDebugBuild, expected, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_ZERO(value, ...) \
    BASE_ASSERT_ZERO(kIsDebugBuild, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_NOT_ZERO(value, ...) \
    BASE_ASSERT_NOT_ZERO(kIsDebugBuild, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_TRUE(value, ...) \
    BASE_ASSERT_TRUE(kIsDebugBuild, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_FALSE(value, ...) \
    BASE_ASSERT_FALSE(kIsDebugBuild, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_NOT_NULL(value, ...) \
    BASE_ASSERT_NOT_NULL(kIsDebugBuild, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_NULL(value, ...) \
    BASE_ASSERT_NULL(kIsDebugBuild, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_LT(val1, val2, ...) \
    BASE_ASSERT_LT(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_LE(val1, val2, ...) \
    BASE_ASSERT_LE(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_GT(val1, val2, ...) \
    BASE_ASSERT_GT(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_GE(val1, val2, ...) \
    BASE_ASSERT_GE(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_STREQ(val1, val2, ...) \
    BASE_ASSERT_STREQ(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT_STRNEQ(val1, val2, ...) \
    BASE_ASSERT_STRNEQ(kIsDebugBuild, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)

/* release build asserts */
#define R_ASSERT_EQ(expected, value, ...) \
    BASE_ASSERT_EQ(true, expected, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_NEQ(expected, value, ...) \
    BASE_ASSERT_NEQ(true, expected, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_ZERO(value, ...) \
    BASE_ASSERT_ZERO(true, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_NOT_ZERO(value, ...) \
    BASE_ASSERT_NOT_ZERO(true, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_TRUE(value, ...) \
    BASE_ASSERT_TRUE(true, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_FALSE(value, ...) \
    BASE_ASSERT_FALSE(true, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_NOT_NULL(value, ...) \
    BASE_ASSERT_NOT_NULL(true, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_NULL(value, ...) \
    BASE_ASSERT_NULL(true, value, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_LT(val1, val2, ...) \
    BASE_ASSERT_LT(true, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_LE(val1, val2, ...) \
    BASE_ASSERT_LE(true, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_GT(val1, val2, ...) \
    BASE_ASSERT_GT(true, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_GE(val1, val2, ...) \
    BASE_ASSERT_GE(true, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_STREQ(val1, val2, ...) \
    BASE_ASSERT_STREQ(true, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)
#define R_ASSERT_STRNEQ(val1, val2, ...) \
    BASE_ASSERT_STRNEQ(true, val1, val2, __ASSERT_FAIL_FUNC __VA_OPT__(, ) __VA_ARGS__)

/**
 * @def STATIC_ASSERT_CONCEPT(TYPE, CONCEPT)
 * @brief A macro to statically assert that a given TYPE satisfies a CONCEPT.
 *
 * @param TYPE The template parameter type to be checked.
 * @param CONCEPT The concept that the TYPE is expected to satisfy.
 *
 * To be used within a class or function body to enforce
 * concept-based constraints on template parameters. Generates a standardized
 * error message upon failure.
 *
 * Reasoning:
 * Adding a 'requires' clause to a definition has the side-effect of adding
 * whatever dependency the clause needed, as a dependency to every forward
 * declaration of the class.
 *
 * Example:
 * ```
 * // Foo.hpp
 *
 * template < typename T >
 * class Foo
 *      requires something_from < Bar >
 * [class definition]
 *
 * // OtherClass.hpp
 *
 * // Now when I need to forward declare 'Foo'
 * // I need to write it as
 *
 * template < typename T >
 * class Foo
 *      requires something_from < Bar >
 *
 * // AND ALSO INCLUDE FULL DEFINITION OF 'Bar'
 * ```
 *
 * Using this macro allows for compile time concept checking (with a bit
 * less precise error message), but allows forward declarations to be
 * self-contained again.
 *
 */
#define STATIC_ASSERT_CONCEPT(TYPE, CONCEPT) \
    static_assert(CONCEPT<TYPE>, #TYPE " must satisfy the " #CONCEPT " concept.")

#endif  // __cplusplus

#endif  // ALKOS_LIBC_INCLUDE_ASSERT_H_
