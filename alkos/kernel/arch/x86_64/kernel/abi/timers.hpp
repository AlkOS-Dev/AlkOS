#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_TIMERS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_TIMERS_HPP_

#include <time.h>
#include <drivers/cmos/rtc.hpp>
#include <extensions/time.hpp>
#include <modules/timing.hpp>
#include <timers.hpp>

WRAP_CALL time_t QuerySystemTime(const timezone& tz)
{
    static constexpr size_t kBuffSize = 64;
    [[maybe_unused]] char buffer[kBuffSize];

    const tm rtcTime = ReadRtcTime();

    TRACE_INFO("Time loaded from CMOS: %s", [&] {
        strftime(buffer, kBuffSize, "%Y-%m-%d %H:%M:%S", &rtcTime);
        return buffer;
    }());

    return ConvertDateTimeToPosix(rtcTime, tz);
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_TIMERS_HPP_
