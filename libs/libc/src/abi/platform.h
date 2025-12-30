#ifndef LIBS_LIBC_SRC_ABI_PLATFORM_H_
#define LIBS_LIBC_SRC_ABI_PLATFORM_H_

#include "defines.h"
#include "sys/time.h"
#include "types.h"

BEGIN_DECL_C

// Panic
NO_RET void __platform_panic(const char *msg);

// Time Related
void __platform_get_clock_value(ClockType type, TimeVal *time, Timezone *time_zone);
u64 __platform_get_clock_ticks_in_second(ClockType type);
void __platform_get_timezone(Timezone *time_zone);

// Debug IO
void __platform_debug_write(const char *buffer);
size_t __platform_debug_read_line(char *buffer, size_t buffer_size);
void __platform_write_console(const char *buffer);

// File Descriptor Related
int __platform_open(const char *pathname, int flags);
int __platform_close(int fd);
ssize_t __platform_read(int fd, void *buf, size_t count);
ssize_t __platform_write(int fd, const void *buf, size_t count);

END_DECL_C

#endif  // LIBS_LIBC_SRC_ABI_PLATFORM_H_
