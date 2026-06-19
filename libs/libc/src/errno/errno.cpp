// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <errno.h>

/* Errno should be thread local */
TODO_THREADING
error_t g_errno = NO_ERROR;

error_t *__access_errno() { return &g_errno; }
