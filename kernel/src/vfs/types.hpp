#ifndef KERNEL_SRC_VFS_TYPES_HPP_
#define KERNEL_SRC_VFS_TYPES_HPP_

#include <types.h>
#include <data_structures/critbit_tree.hpp>

#include "vfs/error.hpp"
#include "vfs/interface.hpp"
#include "vfs/path.hpp"

namespace vfs
{

/**
 * @brief Filesystem callbacks struct for VFS operations.
 *
 * This struct contains function pointers that implement filesystem operations.
 * Each filesystem driver should provide an instance of this struct with
 * appropriate implementations for its operations.
 */

// Callback type for directory listing - receives user context, entry name, and is_dir flag
using ListDirCallback = void (*)(void *user_ctx, const char *name, bool is_dir);

struct Filesystem {
    // ------------------------------
    // Structures
    // ------------------------------

    struct Operations {
        // File operations
        Result<> (*create_file)(void *ctx, const Path &path);
        Result<size_t> (*read_file)(
            void *ctx, const Path &path, void *buffer, size_t size, size_t offset
        );
        Result<size_t> (*write_file)(
            void *ctx, const Path &path, const void *buffer, size_t size, size_t offset
        );
        Result<> (*delete_file)(void *ctx, const Path &path);
        Result<bool> (*file_exists)(void *ctx, const Path &path);
        Result<size_t> (*get_file_size)(void *ctx, const Path &path);

        // Directory operations
        Result<> (*create_directory)(void *ctx, const Path &path);
        Result<> (*remove_directory)(void *ctx, const Path &path);
        void (*list_directory)(
            void *ctx, const Path &path, ListDirCallback callback, void *user_ctx
        );
        Result<bool> (*directory_exists)(void *ctx, const Path &path);

        // General operations
        Result<bool> (*exists)(void *ctx, const Path &path);
        Result<> (*move)(void *ctx, const Path &old_path, const Path &new_path);
    };

    struct Info {
        Type type;
        const char *name;  // Filesystem name (e.g., "FAT32", "FAT16")
    };

    // ------------------------------
    // Creation and Destruction
    // ------------------------------

    Filesystem() = delete;
    explicit Filesystem(void *context, const Operations &operations, const Info &info)
        : context_(context), ops_(operations), info_(info)
    {
    }

    // ------------------------------
    // Filesystem Operations
    // ------------------------------

    // File operations
    FORCE_INLINE_F Result<> CreateFile(const Path &path) const
    {
        return ops_.create_file(context_, path);
    }

    FORCE_INLINE_F Result<size_t> ReadFile(
        const Path &path, void *buffer, size_t size, size_t offset
    ) const
    {
        return ops_.read_file(context_, path, buffer, size, offset);
    }

    FORCE_INLINE_F Result<size_t> WriteFile(
        const Path &path, const void *buffer, size_t size, size_t offset
    ) const
    {
        return ops_.write_file(context_, path, buffer, size, offset);
    }

    FORCE_INLINE_F Result<> DeleteFile(const Path &path) const
    {
        return ops_.delete_file(context_, path);
    }

    FORCE_INLINE_F Result<bool> FileExists(const Path &path) const
    {
        return ops_.file_exists(context_, path);
    }

    FORCE_INLINE_F Result<size_t> GetFileSize(const Path &path) const
    {
        return ops_.get_file_size(context_, path);
    }

    // Directory operations
    FORCE_INLINE_F Result<> CreateDirectory(const Path &path) const
    {
        return ops_.create_directory(context_, path);
    }

    FORCE_INLINE_F Result<> RemoveDirectory(const Path &path) const
    {
        return ops_.remove_directory(context_, path);
    }

    template <typename Callback>
    void ListDirectory(const Path &path, Callback &&callback) const
    {
        ops_.list_directory(
            context_, path,
            [](void *user_ctx, const char *name, bool is_dir) {
                (*static_cast<Callback *>(user_ctx))(name, is_dir);
            },
            &callback
        );
    }

    FORCE_INLINE_F Result<bool> DirectoryExists(const Path &path) const
    {
        return ops_.directory_exists(context_, path);
    }

    // General operations
    FORCE_INLINE_F Result<bool> Exists(const Path &path) const
    {
        return ops_.exists(context_, path);
    }

    FORCE_INLINE_F Result<> Move(const Path &old_path, const Path &new_path) const
    {
        return ops_.move(context_, old_path, new_path);
    }

    FORCE_INLINE_F const Info &GetInfo() const { return info_; }

    private:
    void *context_;  // Filesystem instance
    Operations ops_;
    Info info_;
};

struct MountOptions {
    bool read_only : 1 = false;
};

enum class FileType : u8 {
    File,
    Directory,
};

struct MountPoint {
    Path path;
    MountOptions options;
    Filesystem fs;

    MountPoint() = delete;
    explicit MountPoint(
        const Path &mount_path, MountOptions mount_options, const Filesystem &filesystem
    )
        : path(mount_path), options(mount_options), fs(filesystem)
    {
    }

    ~MountPoint() = default;
};

using Mounts = data_structures::CritBitTree<MountPoint>;

}  // namespace vfs

#endif  // KERNEL_SRC_VFS_TYPES_HPP_
