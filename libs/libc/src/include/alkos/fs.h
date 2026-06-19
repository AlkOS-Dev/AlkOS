// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_FS_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_FS_H_

#include <types.h>

#define ALK_MAX_NAME_LEN 128

typedef enum {
    kFileTypeRegular,
    kFileTypeDirectory,
} FileType;

typedef struct {
    char name[ALK_MAX_NAME_LEN];
    FileType type;
} DirEntry;

typedef struct {
    u64 size;       // File size in bytes
    FileType type;  // File type
} FileInfo;
#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_FS_H_
