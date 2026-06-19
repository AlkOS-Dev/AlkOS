// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_STDLIB_H_
#define LIBS_LIBC_SRC_INCLUDE_STDLIB_H_

#include <defines.h>
#include <stddef.h>
#include <stdint.h>

BEGIN_DECL_C

CONSTEXPR int abs(const int n) { return n < 0 ? -n : n; }

CONSTEXPR long labs(const long n) { return n < 0 ? -n : n; }

CONSTEXPR long long llabs(const long long n) { return n < 0 ? -n : n; }

CONSTEXPR intmax_t imaxabs(const intmax_t n) { return n < 0 ? -n : n; }

// Memory allocation functions
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

// Process control functions
NO_RET void exit(int status);
int atexit(void (*func)(void));

// String conversion functions
int atoi(const char *str);
double atof(const char *str);

// String manipulation functions
char *strdup(const char *s);

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_STDLIB_H_
