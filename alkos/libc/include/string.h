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
 *  Returns the length of the string `str`, but at most `max` characters
 */
size_t strnlen(const char *str, size_t n);

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

/**
 * @brief Copy memory area
 *
 * @param dest Destination memory area
 * @param src Source memory area
 * @param n Number of bytes to copy
 * @return void* Pointer to the destination memory area
 *
 * @note The memory areas must not overlap use memmove() instead
 */
void *memcpy(void *dest, const void *src, size_t n);

/**
 * @brief Copy memory area as if it was first copied to a temporary area and then copied back to
 * dest
 *
 * @param dest Destination memory area
 * @param src Source memory area
 * @param n Number of bytes to copy
 * @return void* Pointer to the destination memory area
 */
void *memmove(void *dest, const void *src, size_t n);

/**
 * @brief Fill memory area with a constant byte
 *
 * @param dest Destination memory area
 * @param c Constant byte
 * @param n Number of bytes to fill
 * @return void* Pointer to the destination memory area
 */
void *memset(void *dest, int c, size_t n);

/**
 * @brief Compare two memory areas
 *
 * @param s1 First memory area
 * @param s2 Second memory area
 * @param n Number of bytes to compare
 * @return int 0 if the memory areas are equal\n
 * \<0 if the first differing byte of s1 is less than the respective one of s2\n
 * \>0 if the first differing byte of s1 is greater than the respective one of s2
 *
 * @note If s1 or s2 is a null pointer, the function returns 0
 * @note if n is 0, the function returns 0
 */
int memcmp(const void *s1, const void *s2, size_t n);

END_DECL_C

#endif  // ALKOS_LIBC_INCLUDE_STRING_H_
