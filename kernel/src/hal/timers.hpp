#ifndef KERNEL_SRC_HAL_TIMERS_HPP_
#define KERNEL_SRC_HAL_TIMERS_HPP_

#include <hal/impl/timers.hpp>

namespace hal
{
/**
 * @brief This function should use some hardware timer to get the current system time.
 *
 * @note It will be used during the boot process and periodically to update the system time and
 *       get rid of any time drift created possibly by inaccurate timers. Returns POSIX time value.
 */
WRAP_CALL time_t QuerySystemTime(const Timezone &tz) { return arch::QuerySystemTime(tz); }

/**
 * @brief this function based on clock infra should pick the best clock source available
 *
 * @note All clock sources should be registered before this function is called.
 */
WRAP_CALL void PickSystemClockSource() { arch::PickSystemClockSource(); }
WRAP_CALL void PickSystemEventClockSource() { arch::PickSystemEventClockSource(); }
}  // namespace hal

#endif  // KERNEL_SRC_HAL_TIMERS_HPP_
