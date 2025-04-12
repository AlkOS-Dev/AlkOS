#ifndef ALKOS_KERNEL_THIRDPARTY_UACPI_UACPI_LIBC_H_
#define ALKOS_KERNEL_THIRDPARTY_UACPI_UACPI_LIBC_H_

#include <stdio.h>
#include <string.h>

#define uacpi_memcpy    memcpy
#define uacpi_memmove   memmove
#define uacpi_memset    memset
#define uacpi_memcmp    memcmp
#define uacpi_strlen    strlen
#define uacpi_strnlen   strnlen
#define uacpi_strcmp    strcmp
#define uacpi_snprintf  snprintf
#define uacpi_vsnprintf vsnprintf

#endif  // ALKOS_KERNEL_THIRDPARTY_UACPI_UACPI_LIBC_H_
