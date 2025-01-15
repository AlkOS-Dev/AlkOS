#ifndef KERNEL_INCLUDE_DEFINES_HPP_
#define KERNEL_INCLUDE_DEFINES_HPP_

/* defines from libc */
#include <defines.h>

// ------------------------------------
// Macro to constexpr conversions
// ------------------------------------

#ifdef __USE_DEBUG_OUTPUT__
static constexpr bool kUseDebugOutput = true;
#else
static constexpr bool kUseDebugOutput = false;
#endif  // __USE_DEBUG_OUTPUT__

#ifdef __USE_DEBUG_TRACES__
static constexpr bool kUseDebugTraces = true;
#else
static constexpr bool kUseDebugTraces = false;
#endif  // __USE_DEBUG_TRACES__

#ifdef __ALKOS_TESTS_BUILD__
static constexpr bool kIsAlkosTestBuild = true;
#else
static constexpr bool kIsAlkosTestBuild = false;
#endif  // __ALKOS_TESTS_BUILD__

#ifdef NDEBUG
static constexpr bool kIsDebugBuild = false;
#else
static constexpr bool kIsDebugBuild = true;
#endif  // NDEBUG

#endif  // KERNEL_INCLUDE_DEFINES_HPP_
