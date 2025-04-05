#ifndef ALKOS_LIBC_INTERNAL_TIME_INTERNAL_HPP_
#define ALKOS_LIBC_INTERNAL_TIME_INTERNAL_HPP_

#include <stdint.h>

[[nodiscard]] uint64_t __GetLocalTimezoneOffsetNs();

[[nodiscard]] uint64_t __GetDstTimezoneOffsetNs();

#endif  // ALKOS_LIBC_INTERNAL_TIME_INTERNAL_HPP_
