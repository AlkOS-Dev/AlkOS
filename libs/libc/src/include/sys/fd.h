#ifndef LIBS_LIBC_SRC_INCLUDE_SYS_FD_H_
#define LIBS_LIBC_SRC_INCLUDE_SYS_FD_H_

typedef int fd_t;

typedef enum {
    kFdFlagRead      = 0x1,
    kFdFlagWrite     = 0x2,
    kFdFlagReadWrite = kFdFlagRead | kFdFlagWrite,
    kFdFlagAppend    = 0x4,
    kFdFlagCreate    = 0x8,
    kFdFlagTruncate  = 0x10,
    kFdFlagNonBlock  = 0x20,
    kFdFlagSync      = 0x40
} FdOpenFlags;

// Standard file descriptors
enum { kFdStdIn = 0, kFdStdOut = 1, kFdStdErr = 2 };

typedef enum { kFdBufferNone = 0, kFdBufferLine = 1, kFdBufferFull = 2 } FdBufferMode;

typedef enum { kFdSeekSet = 0, kFdSeekCurrent = 1, kFdSeekEnd = 2 } FdSeek;

#endif  // LIBS_LIBC_SRC_INCLUDE_SYS_FD_H_
