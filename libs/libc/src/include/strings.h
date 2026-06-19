// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_STRINGS_H_
#define LIBS_LIBC_SRC_INCLUDE_STRINGS_H_

#include <defines.h>

BEGIN_DECL_C

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_STRINGS_H_
