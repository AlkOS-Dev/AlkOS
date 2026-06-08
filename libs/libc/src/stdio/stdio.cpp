#include "stdio.h"
#include "internal/stdio.hpp"

#include <assert.h>
#include <algorithm.hpp>

#include "platform.h"
#include "string.h"

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Flush the internal buffer of a FILE stream
 * @param stream The FILE stream to flush
 * @return 0 on success, EOF on error
 */
static int FlushBuffer(FILE *stream)
{
    if (stream == nullptr) {
        return EOF;
    }

    // If buffer is empty or stream is not writable, nothing to do
    if (stream->buffer_pos == 0) {
        return 0;
    }

    if (!(stream->flags.open_mode & kFdFlagWrite)) {
        return 0;
    }

    // Write the buffered data
    ssize_t written = WriteToFd(stream->fd, stream->buffer, stream->buffer_pos);
    if (written < 0) {
        stream->flags.error = true;
        return EOF;
    }

    // Update file position and reset buffer
    stream->file_pos += written;
    stream->buffer_pos = 0;

    return 0;
}

/**
 * @brief Parse file open mode string
 * @param mode Mode string (e.g., "r", "w", "a", "r+", etc.)
 * @param flags Output parameter for open flags
 * @return true if mode is valid, false otherwise
 */
static bool ParseMode(const char *mode, FdOpenFlags *flags)
{
    if (mode == nullptr || flags == nullptr) {
        return false;
    }

    *flags = static_cast<FdOpenFlags>(0);

    switch (mode[0]) {
        case 'r':
            *flags = kFdFlagRead;
            if (mode[1] == '+') {
                *flags = kFdFlagReadWrite;
            }
            break;
        case 'w':
            *flags = static_cast<FdOpenFlags>(kFdFlagWrite | kFdFlagCreate | kFdFlagTruncate);
            if (mode[1] == '+') {
                *flags =
                    static_cast<FdOpenFlags>(kFdFlagReadWrite | kFdFlagCreate | kFdFlagTruncate);
            }
            break;
        case 'a':
            *flags = static_cast<FdOpenFlags>(kFdFlagWrite | kFdFlagCreate | kFdFlagAppend);
            if (mode[1] == '+') {
                *flags = static_cast<FdOpenFlags>(kFdFlagReadWrite | kFdFlagCreate | kFdFlagAppend);
            }
            break;
        default:
            return false;
    }

    return true;
}

// ============================================================================
// Formatted Output Functions
// ============================================================================

int fprintf(FILE *stream, const char *format, ...)
{
    if (stream == nullptr || format == nullptr) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);

    return result;
}

int vfprintf(FILE *stream, const char *format, va_list va)
{
    if (stream == nullptr || format == nullptr) {
        return -1;
    }

    // Format the string into a buffer
    static constexpr size_t kTempBufferSize = 2048;
    char buffer[kTempBufferSize];
    const int len = vsnprintf(buffer, sizeof(buffer), format, va);
    if (len < 0) {
        return len;
    }

    // Write the formatted string to the stream
    const size_t written =
        fwrite(buffer, 1, std::min(static_cast<size_t>(len), kTempBufferSize), stream);
    return static_cast<int>(written);
}

int printf(const char *format, ...)
{
    if (format == nullptr) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);

    return result;
}

int vprintf(const char *format, va_list va) { return vfprintf(stdout, format, va); }

int puts(const char *s)
{
    if (!s) {
        return EOF;
    }

    int result = printf("%s\n", s);
    return (result < 0) ? EOF : result;
}

int putchar(int c) { return printf("%c", c); }

// ============================================================================
// File Operations
// ============================================================================

FILE *fopen(const char *pathname, const char *mode)
{
    if (pathname == nullptr || mode == nullptr) {
        return nullptr;
    }

    // Parse the mode string
    FdOpenFlags flags;
    if (!ParseMode(mode, &flags)) {
        return nullptr;
    }

    // Allocate a FILE structure
    FILE *file = AllocFile();
    if (file == nullptr) {
        return nullptr;
    }

    // Open the file
    fd_t fd = OpenFd(pathname, flags);
    if (fd < 0) {
        return nullptr;
    }

    // Initialize the FILE structure
    file->fd                    = fd;
    file->flags.open_mode       = flags;
    file->flags.buffer_mode     = kFdBufferNone;  // TODO: Add buffering support
    file->flags.eof             = false;
    file->flags.error           = false;
    file->flags.closed          = false;
    file->flags.is_buffer_owner = false;
    file->file_pos              = 0;

    // Note: Buffer allocation is handled by the caller via setvbuf if needed
    // Default to unbuffered for now
    TODO_USERSPACE_MEMORY_MANAGEMENT
    file->buffer       = nullptr;
    file->buffer_size  = 0;
    file->buffer_pos   = 0;
    file->buffer_level = 0;

    return file;
}

int fclose(FILE *stream)
{
    if (stream == nullptr) {
        return EOF;
    }

    // Don't close standard streams
    if (stream == stdin || stream == stdout || stream == stderr) {
        return EOF;
    }

    if (stream->flags.closed) {
        return EOF;
    }

    // Flush any buffered data
    if (stream->flags.open_mode & kFdFlagWrite) {
        if (FlushBuffer(stream) != 0) {
            stream->flags.error = true;
        }
    }

    // Close the file descriptor
    int result = CloseFd(stream->fd);
    if (result < 0) {
        stream->flags.error = true;
    }

    // Note: Buffer management is left to the caller
    // We don't free buffers here since we don't own them

    // Mark as closed
    stream->flags.closed = true;

    // Free the FILE structure
    FreeFile(stream);

    return (stream->flags.error) ? EOF : 0;
}

// ============================================================================
// Read/Write Operations
// ============================================================================

size_t fread(void *buf, size_t size, size_t count, FILE *stream)
{
    if (buf == nullptr || stream == nullptr || size == 0 || count == 0) {
        return 0;
    }

    if (stream->flags.closed || stream->flags.eof) {
        return 0;
    }

    if (!(stream->flags.open_mode & kFdFlagRead)) {
        stream->flags.error = true;
        return 0;
    }

    size_t total_bytes = size * count;
    size_t bytes_read  = 0;
    byte *dest         = static_cast<byte *>(buf);

    // If unbuffered, read directly
    if (stream->flags.buffer_mode == kFdBufferNone || stream->buffer == nullptr) {
        ssize_t result = ReadFromFd(stream->fd, dest, total_bytes);
        if (result < 0) {
            stream->flags.error = true;
            return 0;
        }
        if (result == 0) {
            stream->flags.eof = true;
        }
        stream->file_pos += result;
        return result / size;
    }

    // Buffered read
    while (bytes_read < total_bytes) {
        // If buffer has data, copy it
        if (stream->buffer_level > 0) {
            size_t available = stream->buffer_level;
            size_t to_copy =
                (total_bytes - bytes_read < available) ? (total_bytes - bytes_read) : available;
            memcpy(dest + bytes_read, stream->buffer + stream->buffer_pos, to_copy);
            stream->buffer_pos += to_copy;
            stream->buffer_level -= to_copy;
            bytes_read += to_copy;
        } else {
            // Buffer is empty, refill it
            ssize_t result = ReadFromFd(stream->fd, stream->buffer, stream->buffer_size);
            if (result < 0) {
                stream->flags.error = true;
                break;
            }
            if (result == 0) {
                stream->flags.eof = true;
                break;
            }
            stream->buffer_pos   = 0;
            stream->buffer_level = result;
            stream->file_pos += result;
        }
    }

    return bytes_read / size;
}

size_t fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
    if (buf == nullptr || stream == nullptr || size == 0 || count == 0) {
        return 0;
    }

    if (stream->flags.closed) {
        return 0;
    }

    if (!(stream->flags.open_mode & kFdFlagWrite)) {
        stream->flags.error = true;
        return 0;
    }

    size_t total_bytes   = size * count;
    size_t bytes_written = 0;
    const byte *src      = static_cast<const byte *>(buf);

    // If unbuffered, write directly
    if (stream->flags.buffer_mode == kFdBufferNone || stream->buffer == nullptr) {
        ssize_t result = WriteToFd(stream->fd, src, total_bytes);
        if (result < 0) {
            stream->flags.error = true;
            return 0;
        }
        stream->file_pos += result;
        return result / size;
    }

    // Buffered write
    while (bytes_written < total_bytes) {
        size_t remaining    = total_bytes - bytes_written;
        size_t buffer_space = stream->buffer_size - stream->buffer_pos;

        if (buffer_space > 0) {
            size_t to_copy = (remaining < buffer_space) ? remaining : buffer_space;
            memcpy(stream->buffer + stream->buffer_pos, src + bytes_written, to_copy);
            stream->buffer_pos += to_copy;
            bytes_written += to_copy;

            // Check for line buffering - flush on newline
            if (stream->flags.buffer_mode == kFdBufferLine) {
                for (size_t i = 0; i < to_copy; i++) {
                    if (src[bytes_written - to_copy + i] == '\n') {
                        if (FlushBuffer(stream) != 0) {
                            stream->flags.error = true;
                            return bytes_written / size;
                        }
                        break;
                    }
                }
            }
        }

        // If buffer is full, flush it
        if (stream->buffer_pos >= stream->buffer_size) {
            if (FlushBuffer(stream) != 0) {
                stream->flags.error = true;
                break;
            }
        }
    }

    // For full buffering, let the buffer fill naturally
    // For line buffering, flushing is already handled above
    // For no buffering, we wrote directly

    return bytes_written / size;
}

int fflush(FILE *stream)
{
    // If stream is NULL, flush all open streams
    if (stream == nullptr) {
        // Flush stdout and stderr
        int result = 0;
        if (FlushBuffer(stdout) != 0) {
            result = EOF;
        }
        if (FlushBuffer(stderr) != 0) {
            result = EOF;
        }
        return result;
    }

    return FlushBuffer(stream);
}

// ============================================================================
// Buffer Control
// ============================================================================

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    if (stream == nullptr) {
        return -1;
    }

    if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF) {
        return -1;
    }

    // Flush existing buffer
    if (stream->flags.open_mode & kFdFlagWrite) {
        FlushBuffer(stream);
    }

    // Reset buffer state
    stream->buffer_pos   = 0;
    stream->buffer_level = 0;

    // Set up new buffer
    stream->flags.buffer_mode = static_cast<FdBufferMode>(mode);
    if (mode == _IONBF) {
        // No buffering
        stream->buffer                = nullptr;
        stream->buffer_size           = 0;
        stream->flags.is_buffer_owner = false;
    } else if (buf != nullptr) {
        // User-provided buffer
        stream->buffer                = reinterpret_cast<byte *>(buf);
        stream->buffer_size           = size;
        stream->flags.is_buffer_owner = false;
    } else {
        TODO_USERSPACE_MEMORY_MANAGEMENT
        R_FAIL_ALWAYS("NOT IMPLEMENTED!");
    }

    return 0;
}

// ============================================================================
// File Positioning
// ============================================================================

int fseek(FILE *stream, long offset, int whence)
{
    if (stream == nullptr) {
        return -1;
    }

    if (stream->flags.closed) {
        return -1;
    }

    if (whence != kFdSeekSet && whence != kFdSeekCurrent && whence != kFdSeekEnd) {
        return -1;
    }

    // Flush write buffer before seeking
    if (stream->flags.open_mode & kFdFlagWrite) {
        if (FlushBuffer(stream) != 0) {
            return -1;
        }
    }

    // Invalidate read buffer
    stream->buffer_pos   = 0;
    stream->buffer_level = 0;

    ssize_t result = __platform_seek(stream->fd, offset, static_cast<FdSeek>(whence));
    if (result < 0) {
        stream->flags.error = true;
        return -1;
    }

    stream->file_pos  = result;
    stream->flags.eof = false;

    return 0;
}

int ferror(FILE *stream)
{
    if (stream == nullptr) {
        return 0;
    }
    return stream->flags.error ? 1 : 0;
}

int feof(FILE *stream)
{
    if (stream == nullptr) {
        return 0;
    }
    return stream->flags.eof ? 1 : 0;
}

void clearerr(FILE *stream)
{
    if (stream == nullptr) {
        return;
    }
    stream->flags.error = false;
    stream->flags.eof   = false;
}

long ftell(FILE *stream)
{
    if (!stream) {
        return -1L;
    }
    return static_cast<long>(stream->file_pos);
}

int remove(const char *pathname) { return __platform_delete_file(pathname); }

int rename(const char *oldname, const char *newname)
{
    return __platform_move_file(oldname, newname);
}

int sscanf(const char *str, const char *format, ...)
{
    if (!str || !format) {
        return -1;
    }

    va_list args;
    va_start(args, format);

    int count     = 0;
    const char *s = str;
    const char *f = format;

    while (*f && *s) {
        if (*f == '%') {
            f++;
            if (*f == '%') {
                // Literal '%'
                if (*s == '%') {
                    s++;
                } else {
                    break;
                }
                f++;
                continue;
            }

            // Skip optional width modifier
            while (*f >= '0' && *f <= '9') {
                f++;
            }

            // Skip length modifiers
            if (*f == 'h' || *f == 'l' || *f == 'L') {
                f++;
            }

            if (*f == 'd' || *f == 'i') {
                // Parse integer
                while (*s == ' ' || *s == '\t' || *s == '\n') {
                    s++;
                }

                int sign = 1;
                int val  = 0;

                if (*s == '-') {
                    sign = -1;
                    s++;
                } else if (*s == '+') {
                    s++;
                }

                while (*s >= '0' && *s <= '9') {
                    val = val * 10 + (*s - '0');
                    s++;
                }

                int *ptr = va_arg(args, int *);
                if (ptr) {
                    *ptr = sign * val;
                    count++;
                }
            } else if (*f == 'u') {
                // Parse unsigned integer
                while (*s == ' ' || *s == '\t' || *s == '\n') {
                    s++;
                }

                unsigned int val = 0;

                while (*s >= '0' && *s <= '9') {
                    val = val * 10 + (*s - '0');
                    s++;
                }

                unsigned int *ptr = va_arg(args, unsigned int *);
                if (ptr) {
                    *ptr = val;
                    count++;
                }
            } else if (*f == 'x' || *f == 'X') {
                // Parse hexadecimal integer
                while (*s == ' ' || *s == '\t' || *s == '\n') {
                    s++;
                }

                unsigned int val = 0;

                while (*s) {
                    int digit;
                    if (*s >= '0' && *s <= '9') {
                        digit = *s - '0';
                    } else if (*s >= 'a' && *s <= 'f') {
                        digit = 10 + (*s - 'a');
                    } else if (*s >= 'A' && *s <= 'F') {
                        digit = 10 + (*s - 'A');
                    } else {
                        break;
                    }
                    val = val * 16 + digit;
                    s++;
                }

                unsigned int *ptr = va_arg(args, unsigned int *);
                if (ptr) {
                    *ptr = val;
                    count++;
                }
            } else if (*f == 's') {
                // Parse string
                while (*s == ' ' || *s == '\t' || *s == '\n') {
                    s++;
                }

                char *ptr = va_arg(args, char *);
                if (ptr) {
                    char *p = ptr;
                    while (*s && *s != ' ' && *s != '\t' && *s != '\n') {
                        *p++ = *s++;
                    }
                    *p = '\0';
                    count++;
                }
            } else if (*f == 'c') {
                // Parse character
                char *ptr = va_arg(args, char *);
                if (ptr) {
                    *ptr = *s++;
                    count++;
                }
            } else if (*f == 'f') {
                // Parse float (basic implementation)
                while (*s == ' ' || *s == '\t' || *s == '\n') {
                    s++;
                }

                double sign = 1.0;
                double val  = 0.0;

                if (*s == '-') {
                    sign = -1.0;
                    s++;
                } else if (*s == '+') {
                    s++;
                }

                while (*s >= '0' && *s <= '9') {
                    val = val * 10.0 + (*s - '0');
                    s++;
                }

                if (*s == '.') {
                    s++;
                    double divisor = 10.0;
                    while (*s >= '0' && *s <= '9') {
                        val += (*s - '0') / divisor;
                        divisor *= 10.0;
                        s++;
                    }
                }

                float *ptr = va_arg(args, float *);
                if (ptr) {
                    *ptr = static_cast<float>(sign * val);
                    count++;
                }
            }

            f++;
        } else if (*f == ' ' || *f == '\t' || *f == '\n') {
            // Whitespace in format matches any whitespace in input
            f++;
            while (*s == ' ' || *s == '\t' || *s == '\n') {
                s++;
            }
        } else {
            // Literal character match
            if (*f == *s) {
                f++;
                s++;
            } else {
                break;
            }
        }
    }

    va_end(args);
    return count;
}
