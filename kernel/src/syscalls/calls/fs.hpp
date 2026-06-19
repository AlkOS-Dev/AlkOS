// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SYSCALLS_CALLS_FS_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_FS_HPP_

#include <defines.h>
#include "modules/vfs.hpp"

namespace Syscall
{
// ------------------------------
// File System Syscalls
// ------------------------------

/**
 * @brief Read directory entries
 * @param path Path to the directory
 * @param entries Buffer to store directory entries
 * @param num_entries Output: actual number of entries read
 * @return Success or error
 */
FORCE_INLINE_F Fs::FdResult<void> SysReadDirectory(
    const vfs::Path &path, std::span<DirEntry> entries, size_t *num_entries
)
{
    auto &vfs = ::VfsModule::Get();

    // Check if directory exists
    auto exists_result = vfs.DirectoryExists(path);
    if (!exists_result.has_value() || !exists_result.value()) {
        return std::unexpected(Fs::FdError::kNotFound);
    }

    size_t count = 0;

    // List directory and populate entries
    vfs.ListDirectory(path, [&](const char *name, bool is_dir) {
        if (count >= entries.size()) {
            return;  // No more space
        }

        auto &entry = entries[count];

        // Copy name
        strncpy(entry.name, name, sizeof(entry.name) - 1);
        entry.name[sizeof(entry.name) - 1] = '\0';

        // Set type
        entry.type = is_dir ? kFileTypeDirectory : kFileTypeRegular;

        count++;
    });

    if (num_entries != nullptr) {
        *num_entries = count;
    }

    return {};
}

/**
 * @brief Get file information
 * @param path Path to the file
 * @param info Output: file information structure
 * @return Success or error
 */
FORCE_INLINE_F Fs::FdResult<void> SysFileInfo(const vfs::Path &path, FileInfo *info)
{
    if (info == nullptr) {
        return std::unexpected(Fs::FdError::kInvalidArgument);
    }

    auto &vfs = ::VfsModule::Get();

    // Check if path exists
    auto exists_result = vfs.Exists(path);
    if (!exists_result.has_value() || !exists_result.value()) {
        return std::unexpected(Fs::FdError::kNotFound);
    }

    // Determine if it's a file or directory
    auto is_dir_result = vfs.DirectoryExists(path);
    if (is_dir_result.has_value() && is_dir_result.value()) {
        info->type = kFileTypeDirectory;
        info->size = 0;
    } else {
        info->type = kFileTypeRegular;

        // Get file size
        auto size_result = vfs.GetFileSize(path);
        if (size_result.has_value()) {
            info->size = size_result.value();
        } else {
            info->size = 0;
        }
    }

    return {};
}

/**
 * @brief Create a directory
 * @param path Path to the directory
 * @return Success or error
 */
FORCE_INLINE_F vfs::Result<> SysCreateDirectory(const vfs::Path &path)
{
    return ::VfsModule::Get().CreateDirectory(path);
}

/**
 * @brief Delete a file
 * @param path Path to the file
 * @return Success or error
 */
FORCE_INLINE_F vfs::Result<> SysDeleteFile(const vfs::Path &path)
{
    return ::VfsModule::Get().DeleteFile(path);
}

/**
 * @brief Move a file
 * @param old_path Current path of the file
 * @param new_path New path for the file
 * @return Success or error
 */
FORCE_INLINE_F vfs::Result<> SysMoveFile(const vfs::Path &old_path, const vfs::Path &new_path)
{
    return ::VfsModule::Get().Move(old_path, new_path);
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_FS_HPP_
