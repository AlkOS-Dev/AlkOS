#include "string.h"
#include "stdlib.h"
#include "strings.h"

size_t strlen(const char *str)
{
    const char *s;
    for (s = str; *s; ++s) continue;
    return s - str;
}

size_t strnlen(const char *str, size_t n)
{
    const char *s;
    for (s = str; *s && static_cast<size_t>(s - str) < n; ++s) continue;
    return s - str;
}

char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;
    while ((*tmp++ = *src++)) continue;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i  = 0;
    char *tmp = dest;
    for (; i != n && (*tmp++ = *src++); ++i) continue;
    for (; i < n; ++i) {
        *tmp++ = '\0';
    }
    return dest;
}

int strcmp(const char *str1, const char *str2)
{
    size_t i       = 0;
    const auto *s1 = reinterpret_cast<const unsigned char *>(str1);
    const auto *s2 = reinterpret_cast<const unsigned char *>(str2);
    while (s1[i] == s2[i] && s1[i]) {
        ++i;
    }
    return s1[i] - s2[i];
}

int strncmp(const char *str1, const char *str2, size_t n)
{
    const auto *s1 = reinterpret_cast<const unsigned char *>(str1);
    const auto *s2 = reinterpret_cast<const unsigned char *>(str2);
    size_t i       = 0;
    while (s1[i] == s2[i] && s1[i] && i < n - 1) {
        ++i;
    }
    return s1[i] - s2[i];
}

char *strcat(char *dest, const char *src)
{
    char *tmp = dest;
    tmp += strlen(dest);
    size_t i = 0;
    while (src[i]) {
        tmp[i] = src[i];
        ++i;
    }
    tmp[i] = '\0';
    return dest;
}

char *strncat(char *dest, const char *src, size_t n)
{
    char *tmp = dest;
    tmp += strlen(dest);
    size_t i = 0;
    while (src[i] && i < n) {
        tmp[i] = src[i];
        ++i;
    }
    tmp[i] = '\0';
    return dest;
}

char *strchr(const char *str, int c)
{
    while (*str) {
        if (*str == c) {
            return const_cast<char *>(str);
        }
        ++str;
    }

    if (c == '\0') {
        return const_cast<char *>(str);
    }

    return nullptr;
}

char *strrchr(const char *str, int c)
{
    const char *last = nullptr;
    while (*str) {
        if (*str == c) {
            last = str;
        }
        ++str;
    }

    if (c == '\0') {
        return const_cast<char *>(str);
    }

    return const_cast<char *>(last);
}

char *strdup(const char *s)
{
    if (!s) {
        return nullptr;
    }

    size_t len = strlen(s) + 1;
    char *dup  = static_cast<char *>(malloc(len));
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}

char *strstr(const char *str1, const char *str2)
{
    size_t len = strlen(str2);
    if (len == 0) {
        return const_cast<char *>(str1);
    }

    while (*str1) {
        if (!strncmp(str1, str2, len)) {
            return const_cast<char *>(str1);
        }
        ++str1;
    }

    return nullptr;
}

static int charIgnoreCase(int c)
{
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

int strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        int c1 = charIgnoreCase(*s1);
        int c2 = charIgnoreCase(*s2);
        if (c1 != c2) {
            return c1 - c2;
        }
        ++s1;
        ++s2;
    }
    return charIgnoreCase(*s1) - charIgnoreCase(*s2);
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    while (n > 0 && *s1 && *s2) {
        int c1 = charIgnoreCase(*s1);
        int c2 = charIgnoreCase(*s2);
        if (c1 != c2) {
            return c1 - c2;
        }
        ++s1;
        ++s2;
        --n;
    }
    if (n == 0) {
        return 0;
    }
    return charIgnoreCase(*s1) - charIgnoreCase(*s2);
}
