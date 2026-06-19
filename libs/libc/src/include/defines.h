// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_DEFINES_H_
#define LIBS_LIBC_SRC_INCLUDE_DEFINES_H_

// ------------------------------
// C++ defines
// ------------------------------

#ifdef __cplusplus

#ifdef __ALKOS_LIBK__
static constexpr bool kIsKernel = true;
#else
static constexpr bool kIsKernel = false;
#endif

#endif  // __cplusplus

// ------------------------------
// Attribute macros
// ------------------------------

/* Prevent the compiler from adding padding to structures */
#define PACK __attribute__((__packed__))

/* Indicate that the function will never return */
#define NO_RET __attribute__((noreturn))

/* Force the compiler to always inline the function */
#define FORCE_INLINE_F inline __attribute__((always_inline))

#define PREVENT_INLINE __attribute__((noinline))

/* Force the compiler to always inline the lambda */
#define FORCE_INLINE_L __attribute__((always_inline))

/* Declare a function as a static inline wrapper */
#define WRAP_CALL static FORCE_INLINE_F

/* Require a function to be inlined for performance reasons */
#define FAST_CALL static FORCE_INLINE_F

/* Marks a function for a compiler to prevent any optimizations */
#define NO_OPT __attribute__((optimize("O0")))

/* Marks a function or variable as unused */
#define UNUSED __attribute__((unused))

/* Marks a function or variable as used (prevents removal) */
#define USED __attribute__((used))

/* Marks a function as naked (no prologue/epilogue) */
#define NAKED __attribute__((naked))

/* Marks a variable or function to be placed in a specific section */
#define SECTION(name) __attribute__((section(#name)))

/* Marks a variable or function as weakly linked */
#define WEAK __attribute__((weak))

// ------------------------------
// Useful macros
// ------------------------------

/* Convert a token into a string */
#define STRINGIFY(x) #x

/* Apply STRINGIFY to expand macros before conversion */
#define TOSTRING(x) STRINGIFY(x)

/* Concatenate two tokens */
#define CONCAT_IMPL(a, b) a##b

/* Apply CONCAT_IMPL to expand macros before concatenation */
#define CONCAT(a, b) CONCAT_IMPL(a, b)

/* Creates a COMPILER LEVEL memory barrier forcing optimizer to not re-order memory accesses */
#define COMPILER_FENCE __asm__ volatile("" : : : "memory");

/* Create reserved structure member */
#define RESERVED(size) byte CONCAT(__reserved_, __COUNTER__)[size] UNUSED

// ------------------------------
// Lang specific defines
// ------------------------------

/* C decl */
#ifdef __cplusplus
#define BEGIN_DECL_C extern "C" {
#define END_DECL_C   }  // extern "C"
#else                   // __cplusplus
#define BEGIN_DECL_C
#define END_DECL_C
#endif  // __cplusplus

/* constexpr */
#ifdef __cplusplus
#define CONSTEXPR static constexpr
#else
#define CONSTEXPR static inline
#endif  // __cplusplus

#endif  // LIBS_LIBC_SRC_INCLUDE_DEFINES_H_
