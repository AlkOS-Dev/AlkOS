#ifndef KERNEL_SRC_SYSCALLS_CALLS_FD_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_FD_HPP_

#include <defines.h>
#include "modules/vfs.hpp"

namespace Syscall
{
// ------------------------------
// File Descriptor Syscalls
// ------------------------------

/**
 * @brief Open a file and return its file descriptor
 * @param path Path to the file
 * @param mode Open mode flags
 * @return File descriptor on success, or error
 */
FORCE_INLINE_F Fs::FdResult<fd_t> SysOpen(const vfs::Path &path, const Fs::OpenMode mode)
{
    return ::VfsModule::Get().GetFdManager().Open(path, mode);
}

/**
 * @brief Close a file descriptor
 * @param fd File descriptor to close
 * @return Success or error
 */
FORCE_INLINE_F Fs::FdResult<> SysClose(fd_t fd)
{
    return ::VfsModule::Get().GetFdManager().Close(fd);
}

/**
 * @brief Read data from a file descriptor
 * @param fd File descriptor to read from
 * @param buffer Buffer to store read data
 * @return Number of bytes read on success, or error
 */
FORCE_INLINE_F Fs::FdResult<size_t> SysRead(fd_t fd, std::span<byte> buffer)
{
    return ::VfsModule::Get().GetFdManager().Read(fd, buffer);
}

/**
 * @brief Write data to a file descriptor
 * @param fd File descriptor to write to
 * @param buffer Buffer containing data to write
 * @return Number of bytes written on success, or error
 */
FORCE_INLINE_F Fs::FdResult<size_t> SysWrite(fd_t fd, std::span<const byte> buffer)
{
    return ::VfsModule::Get().GetFdManager().Write(fd, buffer);
}

/**
 * @brief Seek to a position in a file descriptor
 * @param fd File descriptor to seek
 * @param offset Offset to seek to
 * @param whence Seek mode
 * @return New offset on success, or error
 */
FORCE_INLINE_F Fs::FdResult<i64> SysSeek(fd_t fd, const i64 offset, const Fs::FdSeek whence)
{
    return ::VfsModule::Get().GetFdManager().Seek(fd, offset, whence);
}

/**
 * @brief Duplicate a file descriptor
 * @param fd File descriptor to duplicate
 * @return New file descriptor on success, or error
 */
FORCE_INLINE_F Fs::FdResult<fd_t> SysDup(fd_t fd)
{
    return ::VfsModule::Get().GetFdManager().Duplicate(fd);
}

/**
 * @brief Duplicate a file descriptor to a specific new descriptor
 * @param old_fd File descriptor to duplicate
 * @param new_fd New file descriptor
 * @return New file descriptor on success, or error
 */
FORCE_INLINE_F Fs::FdResult<fd_t> SysDupTo(fd_t old_fd, fd_t new_fd)
{
    return ::VfsModule::Get().GetFdManager().Duplicate(old_fd, new_fd);
}

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

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_FD_HPP_
