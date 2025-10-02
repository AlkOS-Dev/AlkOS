#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_TIMERS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_TIMERS_HPP_

#include <hal/api/timers.hpp>
#include <time.h>
#include <extensions/time.hpp>
#include <trace.hpp>
#include "drivers/cmos/rtc.hpp"

namespace arch
{
WRAP_CALL time_t QuerySystemTime(const timezone &tz)
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
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_TIMERS_HPP_
