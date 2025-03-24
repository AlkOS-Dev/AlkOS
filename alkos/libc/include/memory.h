#ifndef ALKOS_LIBC_INCLUDE_MEMORY_H_
#define ALKOS_LIBC_INCLUDE_MEMORY_H_

#include <defines.h>
#include <stddef.h>

BEGIN_DECL_C
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

#endif  // ALKOS_LIBC_INCLUDE_MEMORY_H_
