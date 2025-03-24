#ifndef ALKOS_KERNEL_INCLUDE_DEFINES_HPP_
#define ALKOS_KERNEL_INCLUDE_DEFINES_HPP_

/* defines from libc */
#include <extensions/defines.hpp>

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

#endif  // ALKOS_KERNEL_INCLUDE_DEFINES_HPP_
