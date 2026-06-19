// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_VFS_HPP_
#define KERNEL_SRC_VFS_HPP_

#include <modules/vfs.hpp>

namespace vfs
{

// ------------------------------
// General VFS Operations
// ------------------------------

/**
 * @brief Mount a filesystem at the specified path.
 */
FORCE_INLINE_F Result<> Mount(const Path &mount_path, MountOptions options, Filesystem &driver)
{
    return VfsModule::Get().Mount(mount_path, options, driver);
}

/**
 * @brief Unmount a filesystem at the specified path.
 */
FORCE_INLINE_F Result<> Unmount(const Path &mount_path)
{
    return VfsModule::Get().Unmount(mount_path);
}

/**
 * @brief Create a new file at the specified path.
 */
FORCE_INLINE_F Result<> CreateFile(const Path &path) { return VfsModule::Get().CreateFile(path); }

/**
 * @brief Read data from a file.
 */
FORCE_INLINE_F Result<size_t> ReadFile(
    const Path &path, void *buffer, size_t size, size_t offset = 0
)
{
    return VfsModule::Get().ReadFile(path, buffer, size, offset);
}

/**
 * @brief Write data to a file.
 */
FORCE_INLINE_F Result<size_t> WriteFile(
    const Path &path, const void *buffer, size_t size, size_t offset = 0
)
{
    return VfsModule::Get().WriteFile(path, buffer, size, offset);
}

/**
 * @brief Delete a file.
 */
FORCE_INLINE_F Result<> DeleteFile(const Path &path) { return VfsModule::Get().DeleteFile(path); }

/**
 * @brief Check if a file exists.
 */
FORCE_INLINE_F Result<bool> FileExists(const Path &path)
{
    return VfsModule::Get().FileExists(path);
}

/**
 * @brief Get the size of a file.
 */
FORCE_INLINE_F Result<size_t> GetFileSize(const Path &path)
{
    return VfsModule::Get().GetFileSize(path);
}

/**
 * @brief Create a new directory.
 */
FORCE_INLINE_F Result<> CreateDirectory(const Path &path)
{
    return VfsModule::Get().CreateDirectory(path);
}

/**
 * @brief Remove an empty directory.
 */
FORCE_INLINE_F Result<> RemoveDirectory(const Path &path)
{
    return VfsModule::Get().RemoveDirectory(path);
}

/**
 * @brief List contents of a directory.
 */
template <typename Callback>
FORCE_INLINE_F void ListDirectory(const Path &path, Callback &&callback)
{
    VfsModule::Get().ListDirectory(path, std::forward<Callback>(callback));
}

/**
 * @brief Check if a directory exists.
 */
FORCE_INLINE_F Result<bool> DirectoryExists(const Path &path)
{
    return VfsModule::Get().DirectoryExists(path);
}

/**
 * @brief Check if a path exists (file or directory).
 */
FORCE_INLINE_F Result<bool> Exists(const Path &path) { return VfsModule::Get().Exists(path); }

/**
 * @brief Rename or move a file/directory.
 */
FORCE_INLINE_F Result<> Move(const Path &old_path, const Path &new_path)
{
    return VfsModule::Get().Move(old_path, new_path);
}

}  // namespace vfs

#endif  // KERNEL_SRC_VFS_HPP_
