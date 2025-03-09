#ifndef TIME_INTERNAL_HPP
#define TIME_INTERNAL_HPP

#include <stdint.h>

[[nodiscard]] uint64_t __GetLocalTimezoneOffsetNs();

[[nodiscard]] uint64_t __GetDstTimezoneOffsetNs();

#endif  // TIME_INTERNAL_HPP
