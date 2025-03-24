#ifndef ALKOS_LIBC_INCLUDE_DEFINES_H_
#define ALKOS_LIBC_INCLUDE_DEFINES_H_

#if !defined(__ALKOS_LIBK__) && !__STDC_HOSTED__
#error "Missing environment definition"
#endif

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

/* Force the compiler to always inline the lambda */
#define FORCE_INLINE_L __attribute__((always_inline))

/* Declare a function as a static inline wrapper */
#define WRAP_CALL static FORCE_INLINE_F

/* Require a function to be inlined for performance reasons */
#define FAST_CALL static FORCE_INLINE_F

/* Marks a function for a compiler to prevent any optimizations */
#define NO_OPT __attribute__((optimize("O0")))

// ------------------------------
// Useful macros
// ------------------------------

/* Convert a token into a string */
#define STRINGIFY(x) #x
/* Apply STRINGIFY to expand macros before conversion */
#define TOSTRING(x) STRINGIFY(x)

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

#endif  // ALKOS_LIBC_INCLUDE_DEFINES_H_
