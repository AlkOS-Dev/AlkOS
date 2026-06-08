#include "doomtype.h"
#include "stddef.h"

#ifndef HAVE_STRINGS_H

int strcasecmp(const char *s1, const char *s2)
{
    size_t i = 0;
    while (true) {
        char c1 = s1[i];
        char c2 = s2[i];

        if (c1 >= 'A' && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if (c2 >= 'A' && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }

        if (c1 != c2 || c1 == '\0') {
            return (unsigned char)c1 - (unsigned char)c2;
        }

        ++i;
    }
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    size_t i = 0;
    while (i < n) {
        char c1 = s1[i];
        char c2 = s2[i];

        if (c1 >= 'A' && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if (c2 >= 'A' && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }

        if (c1 != c2 || c1 == '\0') {
            return (unsigned char)c1 - (unsigned char)c2;
        }

        ++i;
    }

    return 0;
}

#endif
