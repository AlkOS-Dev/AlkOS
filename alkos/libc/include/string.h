#ifndef ALKOS_LIBC_INCLUDE_STRING_H_
#define ALKOS_LIBC_INCLUDE_STRING_H_

#include <defines.h>
#include <stddef.h>

BEGIN_DECL_C
/**
 *  Returns the length of the string `str`
 */
size_t strlen(const char *str);

/**
 *  Copies `src`, including the null terminator, to `dst`
 */
char *strcpy(char *dest, const char *src);

/**
 *  Same as `strcpy`, but copies at most `n` characters
 */
char *strncpy(char *dest, const char *src, size_t n);

/**
 *  Compares two strings.
 *  If the return is 0, the strings are equal.
 *  If the return is less then 0, the first character that does not match has a lower value in s1
 * than s2. If the return is greater then 0, the first character that does not match has a greater
 * value in s1 than in s2
 */
int strcmp(const char *str1, const char *str2);

/**
 *  Same as `strcmp`, but compares at most `n` characters
 */
int strncmp(const char *str1, const char *str2, size_t n);

/**
 *  Appends the string `src` to the end of `dest`
 */
char *strcat(char *dest, const char *src);

/**
 *  Same as `strcat`, but appends at most `n` characters
 */
char *strncat(char *dest, const char *src, size_t n);

/**
 *  Returns a pointer to the first occurence of `c` in string `str`
 */
char *strchr(const char *str, int c);

/**
 *  Returns a pointer to the last occurence of `c` in string `str`
 */
char *strrchr(const char *str, int c);

END_DECL_C

#endif  // ALKOS_LIBC_INCLUDE_STRING_H_
