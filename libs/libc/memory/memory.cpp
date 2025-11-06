#include <stddef.h>

#include "string.h"

void *memcpy(void *dest, const void *src, const size_t n)
{
    if (!dest || !src) {
        return nullptr;
    }

    auto *d       = static_cast<unsigned char *>(dest);
    const auto *s = static_cast<const unsigned char *>(src);

    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

void *memmove(void *dest, const void *src, const size_t n)
{
    if (!dest || !src) {
        return nullptr;
    }

    auto *d       = static_cast<unsigned char *>(dest);
    const auto *s = static_cast<const unsigned char *>(src);

    if (d > s) {
        for (size_t i = n; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    } else {
        for (size_t i = 0; i < n; ++i) {
            d[i] = s[i];
        }
    }
    return dest;
}

void *memset(void *dest, const int c, const size_t n)
{
    if (!dest) {
        return nullptr;
    }

    auto *d = static_cast<unsigned char *>(dest);
    auto uc = static_cast<unsigned char>(c);

    for (size_t i = 0; i < n; ++i) {
        d[i] = uc;
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, const size_t n)
{
    if (!s1 || !s2) {
        return 0;
    }
    if (n == 0) {
        return 0;
    }

    const auto *byte1 = static_cast<const unsigned char *>(s1);
    const auto *byte2 = static_cast<const unsigned char *>(s2);

    for (size_t i = 0; i < n; ++i) {
        if (byte1[i] < byte2[i]) {
            return -1;
        }
        if (byte1[i] > byte2[i]) {
            return 1;
        }
    }
    return 0;
}
