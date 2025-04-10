#ifndef ALKOS_KERNEL_INCLUDE_MODULES_TIMING_CONSTANTS_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_TIMING_CONSTANTS_HPP_

#include <sys/time.h>

namespace timing_constants
{
static constexpr u64 kClockTicksInSecond[]{
    0,       /* reserved */
    1,       /* kTimeUtc */
    1000000, /* Process Clock */
};
static constexpr size_t kClockTicksInSecondSize = sizeof(kClockTicksInSecond) / sizeof(u64);
static_assert(kClockTicksInSecondSize == static_cast<size_t>(ClockType::kLastClockType));

}  // namespace timing_constants

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_TIMING_CONSTANTS_HPP_
