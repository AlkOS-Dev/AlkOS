#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_POWER_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_POWER_H_

#include "alkos/power.h"
#include "defines.h"
#include "platform.h"

FORCE_INLINE_F void Shutdown() { __platform_power(kShutdown); }

FORCE_INLINE_F void Reboot() { __platform_power(kReboot); }

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_POWER_H_
