// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INTERNAL_STDIO_HPP_
#define LIBS_LIBC_SRC_INTERNAL_STDIO_HPP_

#include "stdio.h"
#include "string.h"
#include "todo.h"

// Forward declarations
extern byte _stdin_buffer[BUFSIZ];
extern byte _stdout_buffer[BUFSIZ];
extern byte _stderr_buffer[BUFSIZ];

extern FILE _stdin;
extern FILE _stdout;
extern FILE _stderr;

TODO_USERSPACE_MEMORY_MANAGEMENT
// FILE allocator constants
#define FILE_POOL_SIZE 32  // Maximum number of simultaneously open files

/**
 * @brief Node for FILE free list
 */
struct FileNode {
    FILE file;
    FileNode *next;
};

/**
 * @brief Allocate a FILE structure from the internal pool
 * @return Pointer to FILE structure, or nullptr if pool is exhausted
 */
FILE *AllocFile();

/**
 * @brief Free a FILE structure back to the internal pool
 * @param file Pointer to FILE structure to free
 */
void FreeFile(FILE *file);

/**
 * @brief Initialize standard I/O streams
 */
FORCE_INLINE_F void InitStdio()
{
    _stdin.fd                    = kFdStdIn;
    _stdin.flags.open_mode       = kFdFlagRead;
    _stdin.flags.buffer_mode     = kFdBufferLine;
    _stdin.flags.eof             = false;
    _stdin.flags.error           = false;
    _stdin.flags.closed          = false;
    _stdin.flags.is_buffer_owner = false;
    _stdin.buffer                = _stdin_buffer;
    _stdin.buffer_size           = sizeof(_stdin_buffer);
    _stdin.buffer_pos            = 0;
    _stdin.buffer_level          = 0;
    _stdin.file_pos              = 0;

    _stdout.fd                    = kFdStdOut;
    _stdout.flags.open_mode       = kFdFlagWrite;
    _stdout.flags.buffer_mode     = kFdBufferLine;
    _stdout.flags.eof             = false;
    _stdout.flags.error           = false;
    _stdout.flags.closed          = false;
    _stdout.flags.is_buffer_owner = false;
    _stdout.buffer                = _stdout_buffer;
    _stdout.buffer_size           = sizeof(_stdout_buffer);
    _stdout.buffer_pos            = 0;
    _stdout.buffer_level          = 0;
    _stdout.file_pos              = 0;

    _stderr.fd                    = kFdStdErr;
    _stderr.flags.open_mode       = kFdFlagWrite;
    _stderr.flags.buffer_mode     = kFdBufferNone;
    _stderr.flags.eof             = false;
    _stderr.flags.error           = false;
    _stderr.flags.closed          = false;
    _stderr.flags.is_buffer_owner = false;
    _stderr.buffer                = _stderr_buffer;
    _stderr.buffer_size           = sizeof(_stderr_buffer);
    _stderr.buffer_pos            = 0;
    _stderr.buffer_level          = 0;
    _stderr.file_pos              = 0;
}

#endif  // LIBS_LIBC_SRC_INTERNAL_STDIO_HPP_
