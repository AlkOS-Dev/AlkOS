#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_FORMATS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_FORMATS_HPP_

#include <stddef.h>
#include <stdint.h>
#include "extensions/defines.hpp"
#include "extensions/tuple.hpp"
#include "extensions/type_traits.hpp"

inline void ReverseString(char *str, const size_t len)
{
    if (!len) {
        return;
    }

    for (size_t i = 0, j = len - 1; i < j; i++, j--) {
        char tmp = str[i];
        str[i]   = str[j];
        str[j]   = tmp;
    }
}

inline char *FormatUInt(uintmax_t num, char *str)
{
    size_t i = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i]   = '\0';

        return str;
    }

    for (; num; num /= 10) {
        str[i++] = '0' + num % 10;
    }

    str[i] = '\0';

    ReverseString(str, i);

    return str;
}

inline size_t FormatUIntWithoutNullTerm(uintmax_t num, char *str, const size_t len)
{
    if (len == 0) {
        return 0;
    }

    if (num == 0) {
        *str = '0';
        return 1;
    }

    size_t i = 0;
    for (; i < len && num > 0; num /= 10) {
        str[i++] = '0' + num % 10;
    }
    ReverseString(str, i);
    return i;
}

inline const char *FormatMetricUint(uintmax_t num)
{
    TODO_THREADING
    static char buffer[128];

    static const char *kPrefixes[]       = {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
    static constexpr size_t kNumPrefixes = sizeof(kPrefixes) / sizeof(kPrefixes[0]);

    size_t i = 0;
    for (; i < kNumPrefixes && num >= static_cast<uintmax_t>(1e6); i++) {
        num /= (1 << 10);
    }

    snprintf(buffer, sizeof(buffer), "%lld %s", num, kPrefixes[i]);
    return buffer;
}

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_INTERNAL_FORMATS_HPP_
