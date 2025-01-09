#ifndef LIBC_INCLUDE_TYPES_H_
#define LIBC_INCLUDE_TYPES_H_

#include <stdint.h>
#include <limits.h>

#if SIZE_MAX == UINTMAX_MAX
typedef intmax_t ssize_t;
#elif SIZE_MAX == ULONG_LONG_MAX
typedef long long ssize_t;
#elif SIZE_MAX == ULONG_MAX
typedef long ssize_t;
#elif SIZE_MAX == UINT_MAX
typedef int ssize_t;
#elif SIZE_MAX == USHRT_MAX
typedef short ssize_t;
#elif SIZE_MAX == UCHAR_MAX
typedef signed char ssize_t;
#else
#error "Unsupported ssize_t size"
#endif

#if PTRDIFF_MAX == INTMAX_MAX
typedef uintmax_t uptrdiff_t;
#elif PTRDIFF_MAX == LONG_LONG_MAX
typedef unsigned long long uptrdiff_t;
#elif PTRDIFF_MAX == LONG_MAX
typedef unsigned long uptrdiff_t;
#elif PTRDIFF_MAX == INT_MAX
typedef unsigned int uptrdiff_t;
#elif PTRDIFF_MAX == SHRT_MAX
typedef unsigned short uptrdiff_t;
#elif PTRDIFF_MAX == CHAR_MAX
typedef unsigned char uptrdiff_t;
#else
#error "Unsupported uptrdiff_t size"
#endif

#endif // LIBC_INCLUDE_TYPES_H_
