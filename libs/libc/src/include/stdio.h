#ifndef LIBS_LIBC_SRC_INCLUDE_STDIO_H_
#define LIBS_LIBC_SRC_INCLUDE_STDIO_H_

#include "defines.h"
#include "stdarg.h"
#include "stddef.h"
#include "sys/calls/fd.h"

#define _IOFBF kFdBufferFull
#define _IOLBF kFdBufferLine
#define _IONBF kFdBufferNone

typedef struct {
    // File descriptor
    fd_t fd;  // Underlying file descriptor

    struct {
        FdOpenFlags open_mode : 8;     // Open mode
        FdBufferMode buffer_mode : 4;  // Buffering mode
        bool eof : 1;                  // End-of-file flag
        bool error : 1;                // Error flag
        bool closed : 1;               // True if file is closed
        bool is_buffer_owner : 1;      // True if we own the buffer memory
    } flags;

    // Buffering
    byte *buffer;         // Buffer pointer
    size_t buffer_size;   // Buffer size
    size_t buffer_pos;    // Current position in buffer
    size_t buffer_level;  // Number of valid bytes in buffer

    // File position
    u64 file_pos;  // Current file position (for unbuffered I/O)
} FILE;

BEGIN_DECL_C
/**
 *  Writes formatted output to a character array (*str) up to a maximum amount of characters (size)
 */
int snprintf(char *str, size_t size, const char *format, ...);

/**
 *  Writes formatted data from a variable argument list (va) to a sized buffer (str, size)
 */
int vsnprintf(char *str, size_t size, const char *format, va_list va);

/**
 *  Opens a file and returns a FILE pointer
 */
FILE *fopen(const char *pathname, const char *mode);

/**
 *  Closes a FILE pointer
 */
int fclose(FILE *stream);

/**
 *  Reads data from a FILE pointer into a buffer
 */
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);

/**
 *  Writes data from a buffer to a FILE pointer
 */
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_STDIO_H_
