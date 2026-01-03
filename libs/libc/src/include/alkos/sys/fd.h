#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_FD_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_FD_H_

#include "alkos/fd.h"
#include "defines.h"
#include "platform.h"
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

/**
 * @brief Seek to a position in a file descriptor
 * @param fd File descriptor to seek
 * @param offset Offset to seek to
 * @param whence Reference point for offset
 * @return New offset on success, -1 on failure
 */
FAST_CALL ssize_t SeekFd(fd_t fd, ssize_t offset, FdSeek whence)
{
    return __platform_seek(fd, offset, whence);
}

/**
 * @brief Duplicate a file descriptor
 * @param fd File descriptor to duplicate
 * @return New file descriptor on success, -1 on failure
 */
FAST_CALL fd_t DuplicateFd(fd_t fd) { return __platform_dup(fd); }

/**
 * @brief Duplicate a file descriptor to a specific value
 * @param old_fd File descriptor to duplicate
 * @param new_fd Desired new file descriptor value
 * @return New file descriptor on success, -1 on failure
 */
FAST_CALL fd_t DuplicateFdTo(fd_t old_fd, fd_t new_fd) { return __platform_dup_to(old_fd, new_fd); }

/**
 * @brief Read directory entries
 * @param path Path to the directory
 * @param entries Buffer to store directory entries
 * @param max_entries Maximum number of entries to read
 * @param num_entries Output: actual number of entries read
 * @return 0 on success, -1 on failure
 */
FAST_CALL int ReadDirectory(
    const char *path, DirEntry *entries, size_t max_entries, size_t *num_entries
)
{
    return __platform_read_directory(path, entries, max_entries, num_entries);
}

/**
 * @brief Get file information
 * @param path Path to the file
 * @param info Output: file information structure
 * @return 0 on success, -1 on failure
 */
FAST_CALL int GetFileInfo(const char *path, FileInfo *info)
{
    return __platform_file_info(path, info);
}

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_FD_H_
