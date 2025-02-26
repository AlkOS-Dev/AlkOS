#ifndef LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_
#define LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_

#include <defines.h>
#include <stddef.h>

#define NODSCRD [[nodiscard]]

constexpr size_t operator""_s(const char* str, const size_t len)
{
    size_t result = 0;
    for (size_t i = 0; i < len; ++i) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

constexpr size_t operator""_s(const unsigned long long value) { return value; }

#endif  // LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_
