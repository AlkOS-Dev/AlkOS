#ifndef KERNEL_ABI_TIMERS_HPP_
#define KERNEL_ABI_TIMERS_HPP_

#include <time.h>
#include <defines.hpp>

/**
 * @brief This function should use some hardware timer to get the current system time.
 *
 * @note It will be used during the boot process and periodically to update the system time and
 *       get rid of any time drift created possibly by inaccurate timers. Returns POSIX time value.
 */
WRAP_CALL time_t QuerySystemTime();

#include <abi/timers.hpp>

#endif  // KERNEL_ABI_TIMERS_HPP_
