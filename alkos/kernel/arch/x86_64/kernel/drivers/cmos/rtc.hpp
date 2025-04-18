#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_CMOS_RTC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_CMOS_RTC_HPP_

#include <time.h>
#include "extensions/types.hpp"

static constexpr u16 kOsYear    = 2025;
static constexpr u16 kOsCentury = 20;

tm ReadRtcTime();

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_CMOS_RTC_HPP_
