// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INTERNAL_TIME_INTERNAL_HPP_
#define LIBS_LIBC_SRC_INTERNAL_TIME_INTERNAL_HPP_

#include <stdint.h>

[[nodiscard]] uint64_t __GetLocalTimezoneOffsetNs();

[[nodiscard]] uint64_t __GetDstTimezoneOffsetNs();

#endif  // LIBS_LIBC_SRC_INTERNAL_TIME_INTERNAL_HPP_
