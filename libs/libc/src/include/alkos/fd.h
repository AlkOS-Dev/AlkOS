#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_FD_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_FD_H_

#include <types.h>

typedef int fd_t;

#define ALK_MAX_NAME_LEN 128

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

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_FD_H_
