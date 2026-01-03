#ifndef KERNEL_SRC_FS_VFS_ERROR_HPP_
#define KERNEL_SRC_FS_VFS_ERROR_HPP_

#include <expected.hpp>

namespace vfs
{

enum class VfsError {
    kFileNotFound,
    kDirectoryNotFound,
    kInvalidPath,
    kMountPointNotFound,
    kAlreadyMounted,
    kNotMounted,
    kInvalidArgument,
    kAlreadyExists,
    kNotEmpty,
    kNotADirectory,
    kNotAFile,
    kInvalidName,
    kReadOnly,
    kDiskFull,
    kCrossFilesystemRename,
    kUnknownError,
};

template <typename T = void>
using Result = std::expected<T, VfsError>;

}  // namespace vfs

#endif  // KERNEL_SRC_FS_VFS_ERROR_HPP_
