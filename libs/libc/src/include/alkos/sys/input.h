#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_INPUT_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_INPUT_H_

#include "alkos/input.h"
#include "defines.h"
#include "platform.h"

FORCE_INLINE_F bool GetKeyState(VirtualKey vk) { return __platform_get_key_state(vk); }

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_INPUT_H_
