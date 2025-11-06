#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TIMERS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TIMERS_HPP_

#include <time.h>
#include <extensions/time.hpp>
#include "drivers/cmos/rtc.hpp"
#include "trace_framework.hpp"

namespace arch
{
WRAP_CALL time_t QuerySystemTime(const timezone &tz)
{
    static constexpr size_t kBuffSize = 64;
    [[maybe_unused]] char buffer[kBuffSize];

    const tm rtcTime = ReadRtcTime();

    TRACE_INFO_GENERAL("Time loaded from CMOS: %s", [&] {
        strftime(buffer, kBuffSize, "%Y-%m-%d %H:%M:%S", &rtcTime);
        return buffer;
    }());

    return ConvertDateTimeToPosix(rtcTime, tz);
}

void PickSystemClockSource();

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TIMERS_HPP_
