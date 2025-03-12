#ifndef ALKOS_INCLUDE_MODULES_TIMING_CONSTANTS_HPP_
#define ALKOS_INCLUDE_MODULES_TIMING_CONSTANTS_HPP_

#include <sys/time.h>

#define TIMING_DECL_START \
    namespace timing      \
    {
#define TIMING_DECL_END }
#define USE_TIMING      using namespace timing;

namespace timing_constants
{
static constexpr u64 kClockTicksInSecond[]{
    0,       /* reserved */
    1,       /* kTimeUtc */
    1000000, /* Process Clock */
};
static constexpr size_t kClockTicksInSecondSize = sizeof(kClockTicksInSecond) / sizeof(u64);
static_assert(kClockTicksInSecondSize == static_cast<size_t>(ClockType::kLastClockType));

static constexpr Timezone kUtcTimezone = {
    .west_offset_minutes     = 0,
    .dst_time_offset_minutes = 0,
    .dst_time_start_seconds  = static_cast<u16>(-1),
    .dst_time_end_seconds    = static_cast<u16>(-1),
};

}  // namespace timing_constants

#endif  // ALKOS_INCLUDE_MODULES_TIMING_CONSTANTS_HPP_
