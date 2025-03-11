#ifndef ARCH_X86_64_ABI_TIMERS_HPP_
#define ARCH_X86_64_ABI_TIMERS_HPP_

#include <time.h>
#include <drivers/cmos/rtc.hpp>
#include <extensions/time.hpp>
#include <modules/global_state.hpp>
#include <modules/timing.hpp>
#include <timers.hpp>
#include <todo.hpp>

WRAP_CALL time_t QuerySystemTime(const timezone& tz)
{
    const tm rtcTime = ReadRtcTime();
    return ConvertDateTimeToSeconds(rtcTime, tz);
}

#endif  // ARCH_X86_64_ABI_TIMERS_HPP_
