#ifndef LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_
#define LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_

#include <defines.h>
#include <stddef.h>

#ifdef NDEBUG
static constexpr bool kIsDebugBuild = false;
#else
static constexpr bool kIsDebugBuild = true;
#endif  // NDEBUG

#define NODISCARD [[nodiscard]]

/* 1 or nothing */
#define VARIADIC_MACRO_HAS_ARGS(...) __VA_OPT__(1)

/* 1 if has args, 0 if not */
#define BOOL_VARIADIC_MACRO_HAS_ARGS(...) \
    static_cast<bool>(VARIADIC_MACRO_HAS_ARGS(__VA_ARGS__) + 0)

constexpr size_t operator""_size(const char* str, const size_t len)
{
    size_t result = 0;
    for (size_t i = 0; i < len; ++i) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

constexpr size_t operator""_size(const unsigned long long value) { return value; }

#endif  // LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_
