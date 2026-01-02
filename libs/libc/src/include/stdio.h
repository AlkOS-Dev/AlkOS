#ifndef LIBS_LIBC_SRC_INCLUDE_STDIO_H_
#define LIBS_LIBC_SRC_INCLUDE_STDIO_H_

#include "alkos/sys/fd.h"
#include "defines.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stddef.h"

#define BUFSIZ 4096
#define EOF    (-1)

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
int vsnprintf(char *str, size_t size, const char *format, va_list va);

/**
 *  Writes formatted output to a FILE pointer (stream)
 */
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list va);

/**
 *  Writes formatted output to standard output (stdout)
 */
int printf(const char *format, ...);
int vprintf(const char *format, va_list va);

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
size_t fread(void *buf, size_t size, size_t count, FILE *stream);

/**
 *  Writes data from a buffer to a FILE pointer
 */
size_t fwrite(const void *buf, size_t size, size_t count, FILE *stream);

/**
 *  Flushes the output buffer of a FILE pointer
 */
int fflush(FILE *stream);

/**
 *  Sets the buffering mode for a FILE pointer
 */
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

/**
 *  Sets the position of the FILE pointer
 */
int fseek(FILE *stream, long offset, int whence);

// Standard streams
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_STDIO_H_
