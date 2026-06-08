#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_FS_FS_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_FS_FS_H_

#include "alkos/fs.h"
#include "defines.h"
#include "platform.h"
#include "types.h"

BEGIN_DECL_C

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

/**
 * @brief Create a directory
 * @param path Path to the directory
 * @return 0 on success, -1 on failure
 */
FAST_CALL int CreateDirectory(const char *path) { return __platform_create_directory(path); }

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_FS_FS_H_
