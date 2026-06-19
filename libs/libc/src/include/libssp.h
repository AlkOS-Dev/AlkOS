// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_LIBSSP_H_
#define LIBS_LIBC_SRC_INCLUDE_LIBSSP_H_

#include <defines.h>

BEGIN_DECL_C
void __stack_chk_init();
END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_LIBSSP_H_
