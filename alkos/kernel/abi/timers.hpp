#ifndef ALKOS_KERNEL_ABI_TIMERS_HPP_
#define ALKOS_KERNEL_ABI_TIMERS_HPP_

#include <sys/time.h>
#include <time.h>
#include <defines.hpp>

namespace arch
{
/**
 * @brief This function should use some hardware timer to get the current system time.
 *
 * @note It will be used during the boot process and periodically to update the system time and
 *       get rid of any time drift created possibly by inaccurate timers. Returns POSIX time value.
 */
WRAP_CALL time_t QuerySystemTime(const timezone& tz);
}  // namespace arch

#include <abi/timers.hpp>

#endif  // ALKOS_KERNEL_ABI_TIMERS_HPP_
