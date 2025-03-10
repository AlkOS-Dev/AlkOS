#ifndef ARCH_X86_64_ABI_TIMERS_HPP_
#define ARCH_X86_64_ABI_TIMERS_HPP_

#include <time.h>
#include <drivers/cmos/rtc.hpp>
#include <extensions/time.hpp>
#include <modules/global_state.hpp>
#include <modules/timing.hpp>
#include <timers.hpp>
#include <todo.hpp>

WRAP_CALL time_t QuerySystemTime()
{
    const tm rtcTime = ReadRtcTime();

    if (GetSetting<global_state_constants::SettingsType::kIsDayTimeClockInUTC>()) {
        return ConvertDateTimeToSeconds(rtcTime, timing_constants::kUtcTimezone);
    }

    return ConvertDateTimeToSeconds(rtcTime, TimingModule::Get().GetDayTime().GetTimezone());
}

#endif  // ARCH_X86_64_ABI_TIMERS_HPP_
