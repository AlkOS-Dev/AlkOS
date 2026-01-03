#ifndef KERNEL_SRC_MODULES_TIMING_CONSTANTS_HPP_
#define KERNEL_SRC_MODULES_TIMING_CONSTANTS_HPP_

#include <alkos/time.h>

namespace timing_constants
{
static constexpr u64 kClockTicksInSecond[]{
    0,             /* reserved */
    1,             /* kTimeUtc */
    1'000'000,     /* Process Clock */
    1'000'000'000, /* Process Clock Precise */
};
static constexpr size_t kClockTicksInSecondSize = sizeof(kClockTicksInSecond) / sizeof(u64);
static_assert(kClockTicksInSecondSize == static_cast<size_t>(ClockType::kLastClockType));

}  // namespace timing_constants

#endif  // KERNEL_SRC_MODULES_TIMING_CONSTANTS_HPP_
