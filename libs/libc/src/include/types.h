// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_TYPES_H_
#define LIBS_LIBC_SRC_INCLUDE_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#if SIZE_MAX == UINTMAX_MAX
typedef intmax_t ssize_t;
#elif SIZE_MAX == UINT64_MAX
typedef int64_t ssize_t;
#elif SIZE_MAX == UINT32_MAX
typedef int32_t ssize_t;
#elif SIZE_MAX == UINT16_MAX
typedef int16_t ssize_t;
#elif SIZE_MAX == UINT8_MAX
typedef int8_t ssize_t;
#else
#error "Unsupported ssize_t size"
#endif

typedef uintptr_t uptrdiff_t;
typedef size_t rsize_t;

/* simplified unsigned int types */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u8 byte;

/* simplified int types */
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/* simplified float types */
typedef float f32;
typedef double f64;

typedef uintptr_t uptr;
typedef intptr_t iptr;

#endif  // LIBS_LIBC_SRC_INCLUDE_TYPES_H_
