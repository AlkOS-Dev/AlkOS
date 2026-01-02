#ifndef LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_FD_H_
#define LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_FD_H_

#include "defines.h"
#include "platform.h"
#include "alkos/fd.h"
#include "types.h"

BEGIN_DECL_C

/**
 * @brief Open a file
 * @param pathname Path to the file
 * @param flags Open mode flags
 * @return File descriptor on success, -1 on failure
 */
FAST_CALL fd_t OpenFd(const char *pathname, FdOpenFlags flags)
{
    return __platform_open(pathname, flags);
}

/**
 * @brief Close a file descriptor
 * @param fd File descriptor to close
 * @return 0 on success, -1 on failure
 */
FAST_CALL int CloseFd(fd_t fd) { return __platform_close(fd); }

/**
 * @brief Read from a file descriptor
 * @param fd File descriptor to read from
 * @param buf Buffer to store read data
 * @param count Number of bytes to read
 * @return Number of bytes read on success, -1 on failure
 */
FAST_CALL ssize_t ReadFromFd(fd_t fd, void *buf, size_t count)
{
    return __platform_read(fd, buf, count);
}

/**
 * @brief Write to a file descriptor
 * @param fd File descriptor to write to
 * @param buf Buffer containing data to write
 * @param count Number of bytes to write
 * @return Number of bytes written on success, -1 on failure
 */
FAST_CALL ssize_t WriteToFd(fd_t fd, const void *buf, size_t count)
{
    return __platform_write(fd, buf, count);
}

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_SYS_CALLS_FD_H_
