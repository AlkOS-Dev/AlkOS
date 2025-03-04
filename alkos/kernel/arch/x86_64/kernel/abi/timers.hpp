#ifndef ARCH_X86_64_ABI_TIMERS_HPP_
#define ARCH_X86_64_ABI_TIMERS_HPP_

#include <time.h>
#include <drivers/cmos/rtc.hpp>
#include <timers.hpp>

WRAP_CALL time_t QuerySystemTime()
{
    /* TODO: Implement mktime first */
    // tm rtcTime = ReadRtcTime();
    // return mktime(&rtcTime);
    return {};
}

#endif  // ARCH_X86_64_ABI_TIMERS_HPP_
