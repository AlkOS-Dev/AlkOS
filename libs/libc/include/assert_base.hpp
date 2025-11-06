#ifndef LIBS_LIBC_INCLUDE_ASSERT_BASE_HPP_
#define LIBS_LIBC_INCLUDE_ASSERT_BASE_HPP_

#include <assert.h>
#include <defines.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <todo.h>
#include <formats.hpp>
#include <type_traits.hpp>
#include <types.hpp>
#include <utility.hpp>

// -------------------------------------
// TODO: temporary print mechanism
// -------------------------------------

/* This print should be replaced by some type driven print mechanism */
TODO_BY_THE_END_OF_MILESTONE1
static constexpr size_t kObjToHexBuffSize = 512;

template <typename ObjT>
WRAP_CALL void VerboseAssertDumpObj(const ObjT &obj, char *buffer, size_t buffer_size)
{
    DumpObjToBufferHex(obj, buffer, buffer_size);
}

FAST_CALL void VerboseAssertDumpObj(const bool &obj, char *buffer, const size_t)
{
    strcpy(buffer, obj ? "true" : "false");
}

FAST_CALL void VerboseAssertDumpObj(const char *&obj, char *buffer, const size_t buffer_size)
{
    strncpy(buffer, obj, buffer_size);
}

FAST_CALL void VerboseAssertDumpObj(const char *const &obj, char *buffer, const size_t buffer_size)
{
    strncpy(buffer, obj, buffer_size);
}

template <typename ObjT, const char *format>
FAST_CALL void VerboseAssertDumpObjSnprintf(const ObjT &obj, char *buffer, const size_t buffer_size)
{
    assert(buffer != nullptr);
    assert(buffer_size > 0);
    snprintf(buffer, buffer_size, format, obj);
}

#define DEF_VERB_DUMP(type, format)                                                              \
    FAST_CALL void VerboseAssertDumpObj(const type &obj, char *buffer, const size_t buffer_size) \
    {                                                                                            \
        static constexpr char kFormat[] = format;                                                \
        VerboseAssertDumpObjSnprintf<type, kFormat>(obj, buffer, buffer_size);                   \
    }

// Unsigned integers
DEF_VERB_DUMP(u8, "%u")
DEF_VERB_DUMP(u16, "%u")
DEF_VERB_DUMP(u32, "%u")
DEF_VERB_DUMP(u64, "%llu")

// Signed integers
DEF_VERB_DUMP(i8, "%d")
DEF_VERB_DUMP(i16, "%d")
DEF_VERB_DUMP(i32, "%d")
DEF_VERB_DUMP(i64, "%lld")

// Floating point
DEF_VERB_DUMP(float, "%g")
DEF_VERB_DUMP(double, "%g")
DEF_VERB_DUMP(long double, "%Lg")

// ------------------------------
// Base for verbose asserts
// ------------------------------

static constexpr size_t kFailMsgBuffSize       = 2048;
static constexpr size_t kFullAssertMsgBuffSize = 512 + kFailMsgBuffSize;
static constexpr size_t kUserMessageBuffSize   = 1024;

using ErrorHandlerFn = void (*)(const char *);

template <ErrorHandlerFn Handler>
PREVENT_INLINE static void VerboseAssertDump(const char *msg, const char *file, const char *line)
{
    char full_msg[kFullAssertMsgBuffSize];
    [[maybe_unused]] const int bytes_written = snprintf(
        full_msg, kFullAssertMsgBuffSize, "Check failed at file: %s and line: %s\n%s\n", file, line,
        msg
    );
    assert(
        bytes_written < static_cast<int>(kFullAssertMsgBuffSize) &&
        "VerboseAssertDump buffer fully used!"
    );

    Handler(full_msg);
}

template <
    ErrorHandlerFn Handler, class ExpectedT, class ValueT, class CheckerT, class MsgGetterT,
    class... Args>
PREVENT_INLINE static void VerboseAssertTwoArgBase(
    const ExpectedT &expected, const ValueT &value, CheckerT checker, MsgGetterT msg_getter,
    const char *expected_str, const char *value_str, const char *file, const char *line,
    Args... args
)
{
    if (!checker(expected, value)) [[unlikely]] {
        char fail_msg[kFailMsgBuffSize];
        char e_obj[kObjToHexBuffSize];
        char v_obj[kObjToHexBuffSize];
        char *msg_buff  = fail_msg;
        size_t msg_size = kFailMsgBuffSize;

        if constexpr (sizeof...(args) > 0) {
            const size_t offset = snprintf(fail_msg, kFailMsgBuffSize, args...);
            fail_msg[offset]    = '\n';
            msg_buff            = fail_msg + offset + 1;
            msg_size            = kFailMsgBuffSize - offset;

            assert(offset < kFailMsgBuffSize && "VerboseAssertTwoArgBase buffer fully used!");
        }

        VerboseAssertDumpObj(expected, e_obj, kObjToHexBuffSize);
        VerboseAssertDumpObj(value, v_obj, kObjToHexBuffSize);

        msg_getter(msg_buff, msg_size, expected_str, value_str, e_obj, v_obj);
        VerboseAssertDump<Handler>(fail_msg, file, line);
    }
}

template <ErrorHandlerFn Handler, class ValueT, class CheckerT, class MsgGetterT, class... Args>
PREVENT_INLINE static void VerboseAssertOneArgBase(
    const ValueT &value, CheckerT checker, MsgGetterT msg_getter, const char *value_str,
    const char *file, const char *line, Args... args
)
{
    if (!checker(value)) [[unlikely]] {
        char fail_msg[kFailMsgBuffSize];
        char v_obj[kObjToHexBuffSize];
        char *msg_buff  = fail_msg;
        size_t msg_size = kFailMsgBuffSize;

        if constexpr (sizeof...(args) > 0) {
            const size_t offset = snprintf(fail_msg, kFailMsgBuffSize, args...);
            fail_msg[offset]    = '\n';
            msg_buff            = fail_msg + offset + 1;
            msg_size            = kFailMsgBuffSize - offset;

            assert(offset < kFailMsgBuffSize && "VerboseAssertOneArgBase buffer fully used!");
        }

        VerboseAssertDumpObj(value, v_obj, kObjToHexBuffSize);

        msg_getter(msg_buff, msg_size, value_str, v_obj);
        VerboseAssertDump<Handler>(fail_msg, file, line);
    }
}

// ------------------------------
// EQ Assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ExpectedT, class ValueT, class... Args>
FAST_CALL void VerboseAssertEq(
    const ExpectedT &expected, const ValueT &value, const char *expected_str, const char *value_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        expected, value,
        [](const ExpectedT &e, const ValueT &v) FORCE_INLINE_L {
            return e == v;
        },
        [](char *msg, const int size, const char *e_str, const char *v_str, const char *e_dump,
           const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (EQ)!\n"
                "Actual value does not match the expected value.\n"
                "Expected value: %s\n"
                "Which is: %s\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                e_str, e_dump, v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertEq buffer fully used!");
        },
        expected_str, value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// NEQ Assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ExpectedT, class ValueT, class... Args>
FAST_CALL void VerboseAssertNeq(
    const ExpectedT &expected, const ValueT &value, const char *expected_str, const char *value_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        expected, value,
        [](const ExpectedT &e, const ValueT &v) FORCE_INLINE_L {
            return e != v;
        },
        [](char *msg, const int size, const char *e_str, const char *v_str, const char *e_dump,
           const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (NEQ)!\n"
                "Actual value does matches the expected value, when it shouldn't.\n"
                "Expected value: %s\n"
                "Which is: %s\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                e_str, e_dump, v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertNeq buffer fully used!");
        },
        expected_str, value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// Zero Assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ValueT, class... Args>
FAST_CALL void VerboseAssertZero(
    const ValueT &value, const char *value_str, const char *file, const char *line, Args... args
)
{
    VerboseAssertOneArgBase<Handler>(
        value,
        [](const ValueT &v) FORCE_INLINE_L {
            return v == static_cast<ValueT>(0);
        },
        [](char *msg, const int size, const char *v_str, const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (ZERO)!\n"
                "Given value was supposed to be equal to 0!\n"
                "Expected value: 0\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertZero buffer fully used!");
        },
        value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// Not Zero Assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ValueT, class... Args>
FAST_CALL void VerboseAssertNotZero(
    const ValueT &value, const char *value_str, const char *file, const char *line, Args... args
)
{
    VerboseAssertOneArgBase<Handler>(
        value,
        [](const ValueT &v) FORCE_INLINE_L {
            return v != static_cast<ValueT>(0);
        },
        [](char *msg, const int size, const char *v_str, const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (NOT ZERO)!\n"
                "Given value was not supposed to be equal to 0!\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertNotZero buffer fully used!");
        },
        value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// TRUE assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ValueT, class... Args>
FAST_CALL void VerboseAssertTrue(
    const ValueT &value, const char *value_str, const char *file, const char *line, Args... args
)
{
    VerboseAssertOneArgBase<Handler>(
        value,
        [](const ValueT &v) FORCE_INLINE_L {
            return static_cast<bool>(v) == true;
        },
        [](char *msg, const int size, const char *v_str, const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (TRUE)!\n"
                "Given value was supposed to be equal to true!\n"
                "Expected value: true\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertTrue buffer fully used!");
        },
        value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// FALSE assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ValueT, class... Args>
FAST_CALL void VerboseAssertFalse(
    const ValueT &value, const char *value_str, const char *file, const char *line, Args... args
)
{
    VerboseAssertOneArgBase<Handler>(
        value,
        [](const ValueT &v) FORCE_INLINE_L {
            return static_cast<bool>(v) == false;
        },
        [](char *msg, const int size, const char *v_str, const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (FALSE)!\n"
                "Given value was supposed to be equal to false!\n"
                "Expected value: false\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertFalse buffer fully used!");
        },
        value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// NOT_NULL assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ValueT, class... Args>
FAST_CALL void VerboseAssertNotNull(
    const ValueT &value, const char *value_str, const char *file, const char *line, Args... args
)
{
    VerboseAssertOneArgBase<Handler>(
        value,
        [](const ValueT &v) FORCE_INLINE_L {
            return v != nullptr;
        },
        [](char *msg, const int size, const char *v_str, const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (NOT_NULL)!\n"
                "Given value was supposed to be not null!\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertNotNull buffer fully used!");
        },
        value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// NULL assert base
// ------------------------------

template <ErrorHandlerFn Handler, class ValueT, class... Args>
FAST_CALL void VerboseAssertNull(
    const ValueT &value, const char *value_str, const char *file, const char *line, Args... args
)
{
    VerboseAssertOneArgBase<Handler>(
        value,
        [](const ValueT &v) FORCE_INLINE_L {
            return v == nullptr;
        },
        [](char *msg, const int size, const char *v_str, const char *v_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (NULL)!\n"
                "Given value was supposed to be null!\n"
                "Actual value: %s\n"
                "Which is: %s\n",
                v_str, v_dump
            );
            assert(bytes_written < size && "VerboseAssertNull buffer fully used!");
        },
        value_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// LT (Less Than) assert base
// ------------------------------

template <ErrorHandlerFn Handler, class Val1T, class Val2T, class... Args>
FAST_CALL void VerboseAssertLt(
    const Val1T &val1, const Val2T &val2, const char *val1_str, const char *val2_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        val1, val2,
        [](const Val1T &v1, const Val2T &v2) FORCE_INLINE_L {
            return v1 < v2;
        },
        [](char *msg, const int size, const char *v1_str, const char *v2_str, const char *v1_dump,
           const char *v2_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (LT)!\n"
                "First value should be less than second value!\n"
                "First value: %s\n"
                "Which is: %s\n"
                "Second value: %s\n"
                "Which is: %s\n",
                v1_str, v1_dump, v2_str, v2_dump
            );
            assert(bytes_written < size && "VerboseAssertLt buffer fully used!");
        },
        val1_str, val2_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// LE (Less Than or Equal) assert base
// ------------------------------

template <ErrorHandlerFn Handler, class Val1T, class Val2T, class... Args>
FAST_CALL void VerboseAssertLe(
    const Val1T &val1, const Val2T &val2, const char *val1_str, const char *val2_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        val1, val2,
        [](const Val1T &v1, const Val2T &v2) FORCE_INLINE_L {
            return v1 <= v2;
        },
        [](char *msg, const int size, const char *v1_str, const char *v2_str, const char *v1_dump,
           const char *v2_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (LE)!\n"
                "First value should be less than or equal to second value!\n"
                "First value: %s\n"
                "Which is: %s\n"
                "Second value: %s\n"
                "Which is: %s\n",
                v1_str, v1_dump, v2_str, v2_dump
            );
            assert(bytes_written < size && "VerboseAssertLe buffer fully used!");
        },
        val1_str, val2_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// GT (Greater Than) assert base
// ------------------------------

template <ErrorHandlerFn Handler, class Val1T, class Val2T, class... Args>
FAST_CALL void VerboseAssertGt(
    const Val1T &val1, const Val2T &val2, const char *val1_str, const char *val2_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        val1, val2,
        [](const Val1T &v1, const Val2T &v2) FORCE_INLINE_L {
            return v1 > v2;
        },
        [](char *msg, const int size, const char *v1_str, const char *v2_str, const char *v1_dump,
           const char *v2_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (GT)!\n"
                "First value should be greater than second value!\n"
                "First value: %s\n"
                "Which is: %s\n"
                "Second value: %s\n"
                "Which is: %s\n",
                v1_str, v1_dump, v2_str, v2_dump
            );
            assert(bytes_written < size && "VerboseAssertGt buffer fully used!");
        },
        val1_str, val2_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// GE (Greater Than or Equal) assert base
// ------------------------------

template <ErrorHandlerFn Handler, class Val1T, class Val2T, class... Args>
FAST_CALL void VerboseAssertGe(
    const Val1T &val1, const Val2T &val2, const char *val1_str, const char *val2_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        val1, val2,
        [](const Val1T &v1, const Val2T &v2) FORCE_INLINE_L {
            return v1 >= v2;
        },
        [](char *msg, const int size, const char *v1_str, const char *v2_str, const char *v1_dump,
           const char *v2_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (GE)!\n"
                "First value should be greater than or equal to second value!\n"
                "First value: %s\n"
                "Which is: %s\n"
                "Second value: %s\n"
                "Which is: %s\n",
                v1_str, v1_dump, v2_str, v2_dump
            );
            assert(bytes_written < size && "VerboseAssertGe buffer fully used!");
        },
        val1_str, val2_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// String Equal assert base
// ------------------------------

template <ErrorHandlerFn Handler, class... Args>
FAST_CALL void VerboseAssertStrEq(
    const char *val1, const char *val2, const char *val1_str, const char *val2_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        val1, val2,
        [](const char *v1, const char *v2) FORCE_INLINE_L {
            return strcmp(v1, v2) == 0;
        },
        [](char *msg, const int size, const char *v1_str, const char *v2_str, const char *v1_dump,
           const char *v2_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Check failed (STREQ)!\n"
                "Strings should be equal!\n"
                "First string: %s\n"
                "Which is: %s\n"
                "Second string: %s\n"
                "Which is: %s\n",
                v1_str, v1_dump, v2_str, v2_dump
            );
            assert(bytes_written < size && "VerboseAssertStrEq buffer fully used!");
        },
        val1_str, val2_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// String Not Equal assert base
// ------------------------------

template <ErrorHandlerFn Handler, class... Args>
FAST_CALL void VerboseAssertStrNeq(
    const char *val1, const char *val2, const char *val1_str, const char *val2_str,
    const char *file, const char *line, Args... args
)
{
    VerboseAssertTwoArgBase<Handler>(
        val1, val2,
        [](const char *v1, const char *v2) FORCE_INLINE_L {
            return strcmp(v1, v2) != 0;
        },
        [](char *msg, const int size, const char *v1_str, const char *v2_str, const char *v1_dump,
           const char *v2_dump) FORCE_INLINE_L {
            [[maybe_unused]] const int bytes_written = snprintf(
                msg, size,
                "Assertion failed (STRNEQ)!\n"
                "Strings should not be equal!\n"
                "First string: %s\n"
                "Which is: %s\n"
                "Second string: %s\n"
                "Which is: %s\n",
                v1_str, v1_dump, v2_str, v2_dump
            );
            assert(bytes_written < size && "VerboseAssertStrNeq buffer fully used!");
        },
        val1_str, val2_str, file, line, std::forward<Args>(args)...
    );
}

// ------------------------------
// Macro wrappers
// ------------------------------

#define BASE_ASSERT_EQ(is_active, expected, value, handler, ...)        \
    if constexpr (is_active)                                            \
    VerboseAssertEq<handler>(                                           \
        expected, value, TOSTRING(expected), TOSTRING(value), __FILE__, \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__                   \
    )
#define BASE_ASSERT_NEQ(is_active, expected, value, handler, ...)       \
    if constexpr (is_active)                                            \
    VerboseAssertNeq<handler>(                                          \
        expected, value, TOSTRING(expected), TOSTRING(value), __FILE__, \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__                   \
    )
#define BASE_ASSERT_TRUE(is_active, value, handler, ...)                                \
    if constexpr (is_active)                                                            \
    VerboseAssertTrue<handler>(                                                         \
        value, TOSTRING(value), __FILE__, TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__ \
    )
#define BASE_ASSERT_ZERO(is_active, value, handler, ...)                                \
    if constexpr (is_active)                                                            \
    VerboseAssertZero<handler>(                                                         \
        value, TOSTRING(value), __FILE__, TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__ \
    )
#define BASE_ASSERT_NOT_ZERO(is_active, value, handler, ...)                            \
    if constexpr (is_active)                                                            \
    VerboseAssertNotZero<handler>(                                                      \
        value, TOSTRING(value), __FILE__, TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__ \
    )
#define BASE_ASSERT_FALSE(is_active, value, handler, ...)                               \
    if constexpr (is_active)                                                            \
    VerboseAssertFalse<handler>(                                                        \
        value, TOSTRING(value), __FILE__, TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__ \
    )
#define BASE_ASSERT_NOT_NULL(is_active, value, handler, ...)                            \
    if constexpr (is_active)                                                            \
    VerboseAssertNotNull<handler>(                                                      \
        value, TOSTRING(value), __FILE__, TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__ \
    )
#define BASE_ASSERT_NULL(is_active, value, handler, ...)                                \
    if constexpr (is_active)                                                            \
    VerboseAssertNull<handler>(                                                         \
        value, TOSTRING(value), __FILE__, TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__ \
    )
#define BASE_ASSERT_LT(is_active, val1, val2, handler, ...)   \
    if constexpr (is_active)                                  \
    VerboseAssertLt<handler>(                                 \
        val1, val2, TOSTRING(val1), TOSTRING(val2), __FILE__, \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__         \
    )
#define BASE_ASSERT_LE(is_active, val1, val2, handler, ...)   \
    if constexpr (is_active)                                  \
    VerboseAssertLe<handler>(                                 \
        val1, val2, TOSTRING(val1), TOSTRING(val2), __FILE__, \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__         \
    )
#define BASE_ASSERT_GT(is_active, val1, val2, handler, ...)   \
    if constexpr (is_active)                                  \
    VerboseAssertGt<handler>(                                 \
        val1, val2, TOSTRING(val1), TOSTRING(val2), __FILE__, \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__         \
    )
#define BASE_ASSERT_GE(is_active, val1, val2, handler, ...)   \
    if constexpr (is_active)                                  \
    VerboseAssertGe<handler>(                                 \
        val1, val2, TOSTRING(val1), TOSTRING(val2), __FILE__, \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__         \
    )
#define BASE_ASSERT_STREQ(is_active, val1, val2, handler, ...) \
    if constexpr (is_active)                                   \
    VerboseAssertStrEq<handler>(                               \
        val1, val2, TOSTRING(val1), TOSTRING(val2), __FILE__,  \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__          \
    )
#define BASE_ASSERT_STRNEQ(is_active, val1, val2, handler, ...) \
    if constexpr (is_active)                                    \
    VerboseAssertStrNeq<handler>(                               \
        val1, val2, TOSTRING(val1), TOSTRING(val2), __FILE__,   \
        TOSTRING(__LINE__) __VA_OPT__(, ) __VA_ARGS__           \
    )

#endif  // LIBS_LIBC_INCLUDE_ASSERT_BASE_HPP_
