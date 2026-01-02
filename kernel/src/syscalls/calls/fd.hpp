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
    auto result = ::VfsModule::Get().GetFdManager().Open(path, mode);
    RET_UNEXPECTED_IF_ERR(result);
    return result.value();
}

/**
 * @brief Close a file descriptor
 * @param fd File descriptor to close
 * @return Success or error
 */
FORCE_INLINE_F Fs::FdResult<> SysClose(fd_t fd)
{
    auto result = ::VfsModule::Get().GetFdManager().Close(fd);
    RET_UNEXPECTED_IF_ERR(result);
    return {};
}

/**
 * @brief Read data from a file descriptor
 * @param fd File descriptor to read from
 * @param buffer Buffer to store read data
 * @return Number of bytes read on success, or error
 */
FORCE_INLINE_F Fs::FdResult<size_t> SysRead(fd_t fd, std::span<byte> buffer)
{
    auto result = ::VfsModule::Get().GetFdManager().Read(fd, buffer);
    RET_UNEXPECTED_IF_ERR(result);
    return result.value();
}

/**
 * @brief Write data to a file descriptor
 * @param fd File descriptor to write to
 * @param buffer Buffer containing data to write
 * @return Number of bytes written on success, or error
 */
FORCE_INLINE_F Fs::FdResult<size_t> SysWrite(fd_t fd, std::span<const byte> buffer)
{
    auto result = ::VfsModule::Get().GetFdManager().Write(fd, buffer);
    RET_UNEXPECTED_IF_ERR(result);
    return result.value();
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
    auto result = ::VfsModule::Get().GetFdManager().Seek(fd, offset, whence);
    RET_UNEXPECTED_IF_ERR(result);
    return result.value();
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_FD_HPP_
