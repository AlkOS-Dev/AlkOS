#ifndef LIBC_INCLUDE_STDLIB_H_
#define LIBC_INCLUDE_STDLIB_H_

#include <defines.h>
#include <stddef.h>
#include <stdint.h>

BEGIN_DECL_C
CONSTEXPR int abs(const int n) { return n < 0 ? -n : n; }

CONSTEXPR long labs(const long n) { return n < 0 ? -n : n; }

CONSTEXPR long long llabs(const long long n) { return n < 0 ? -n : n; }

CONSTEXPR intmax_t imaxabs(const intmax_t n) { return n < 0 ? -n : n; }

END_DECL_C

#endif  // LIBC_INCLUDE_STDLIB_H_
