#ifndef KERNEL_SRC_MODULES_VFS_HPP_
#define KERNEL_SRC_MODULES_VFS_HPP_

#include <template_lib.hpp>
#include <vfs/types.hpp>
#include "boot_args.hpp"
#include "fs/file_descriptor.hpp"
#include "modules/helpers.hpp"

namespace internal
{
class VfsModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    explicit VfsModule(const BootArguments &args) noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    DEFINE_MODULE_FIELD(vfs, Mounts);
    DEFINE_MODULE_FIELD(Fs, FdManager);

    // ------------------------------
    // Mount Point Management
    // ------------------------------

    public:
    /**
     * @brief Mount a filesystem at the specified path.
     *
     * @param mount_path Absolute path where filesystem should be mounted
     * @param options Mount options (e.g., read-only)
     * @param driver Handle to the filesystem driver
     * @return Result Success or error
     */
    vfs::Result<> Mount(
        const vfs::Path &mount_path, vfs::MountOptions options, vfs::Filesystem &driver
    );

    /**
     * @brief Unmount a filesystem at the specified path.
     *
     * @param mount_path Path of the mount point to unmount
     * @return Result Success or error
     */
    vfs::Result<> Unmount(const vfs::Path &mount_path);

    /**
     * @brief Find the mountpoint for a given path using longest prefix matching.
     *
     * @param path Path to resolve
     * @return Optional pointer to the MountPoint, or empty if no mount found
     */
    vfs::Result<vfs::MountPoint *> FindMountPoint(const vfs::Path &path);

    // ------------------------------
    // File Operations
    // ------------------------------

    /**
     * @brief Create a new file at the specified path.
     *
     * @param path Absolute path of the file to create
     * @return Result Success or error
     */
    vfs::Result<> CreateFile(const vfs::Path &path);

    /**
     * @brief Read data from a file.
     *
     * @param path Absolute path of the file
     * @param buffer Buffer to read data into
     * @param size Number of bytes to read
     * @param offset Offset in the file to start reading from
     * @return Result Number of bytes read or error
     */
    vfs::Result<size_t> ReadFile(const vfs::Path &path, void *buffer, size_t size, size_t offset);

    /**
     * @brief Write data to a file.
     *
     * @param path Absolute path of the file
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     * @param offset Offset in the file to start writing at
     * @return Result Number of bytes written or error
     */
    vfs::Result<size_t> WriteFile(
        const vfs::Path &path, const void *buffer, size_t size, size_t offset
    );

    /**
     * @brief Delete a file.
     *
     * @param path Absolute path of the file to delete
     * @return Result Success or error
     */
    vfs::Result<> DeleteFile(const vfs::Path &path);

    /**
     * @brief Check if a file exists.
     *
     * @param path Absolute path to check
     * @return Result True if file exists, false otherwise, or error
     */
    vfs::Result<bool> FileExists(const vfs::Path &path);

    /**
     * @brief Get the size of a file.
     *
     * @param path Absolute path of the file
     * @return Result File size in bytes or error
     */
    vfs::Result<size_t> GetFileSize(const vfs::Path &path);

    // ------------------------------
    // Directory Operations
    // ------------------------------

    /**
     * @brief Create a new directory.
     *
     * @param path Absolute path of the directory to create
     * @return Result Success or error
     */
    vfs::Result<> CreateDirectory(const vfs::Path &path);

    /**
     * @brief Remove an empty directory.
     *
     * @param path Absolute path of the directory to remove
     * @return Result Success or error
     */
    vfs::Result<> RemoveDirectory(const vfs::Path &path);

    /**
     * @brief List contents of a directory.
     *
     * @param path Absolute path of the directory
     * @param callback Function called for each entry with (name, is_directory)
     */
    template <typename Callback>
    void ListDirectory(const vfs::Path &path, Callback &&callback)
    {
        auto mount_result = FindMountPoint(path);
        if (!mount_result.has_value()) {
            return;
        }

        vfs::MountPoint *mount  = mount_result.value();
        vfs::Path relative_path = GetRelativePath_(path, mount->path);
        mount->fs.ListDirectory(relative_path, std::forward<Callback>(callback));
    }

    /**
     * @brief Check if a directory exists.
     *
     * @param path Absolute path to check
     * @return Result True if directory exists, false otherwise, or error
     */
    vfs::Result<bool> DirectoryExists(const vfs::Path &path);

    // ------------------------------
    // General Operations
    // ------------------------------

    /**
     * @brief Check if a path exists (file or directory).
     *
     * @param path Absolute path to check
     * @return Result True if exists, false otherwise, or error
     */
    vfs::Result<bool> Exists(const vfs::Path &path);

    /**
     * @brief Rename or move a file/directory.
     *
     * @param old_path Current absolute path
     * @param new_path New absolute path
     * @return Result Success or error
     */
    vfs::Result<> Move(const vfs::Path &old_path, const vfs::Path &new_path);

    private:
    /**
     * @brief Convert an absolute path to a path relative to the mount point.
     *
     * @param absolute_path The full absolute path
     * @param mount_path The mount point path
     * @return Path relative to the mount point
     */
    static vfs::Path GetRelativePath_(const vfs::Path &absolute_path, const vfs::Path &mount_path);
};

}  // namespace internal

using VfsModule = template_lib::StaticSingleton<internal::VfsModule>;

#endif  // KERNEL_SRC_MODULES_VFS_HPP_
