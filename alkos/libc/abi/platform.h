#ifndef ALKOS_LIBC_INTERNAL_PLATFORM_H_
#define ALKOS_LIBC_INTERNAL_PLATFORM_H_

#include "defines.h"
#include <extensions/types.hpp> 
#include <sys/time.h>           

BEGIN_DECL_C

// Panic
NO_RET void __platform_panic(const char* msg);

// Time Related
void __platform_get_clock_value(ClockType type, TimeVal* time, Timezone* time_zone);
u64 __platform_get_clock_ticks_in_second(ClockType type);
void __platform_get_timezone(Timezone* time_zone);

// Debug IO
void __platform_debug_write(const char* buffer);
size_t __platform_debug_read_line(char* buffer, size_t buffer_size);

END_DECL_C

#endif  // ALKOS_LIBC_INTERNAL_PLATFORM_H_
