// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "vfs.hpp"
#include "../vfs.hpp"

#include <autogen/feature_flags.h>
#include <fs/vfs/fat/fat16.hpp>
#include <fs/vfs/io/in_memory.hpp>
#include <mem/heap.hpp>
#include <mem/types.hpp>
#include <trace_framework.hpp>
#include "boot_args.hpp"
#include "internal/macros.hpp"

using namespace vfs;

// ------------------------------
// Global ramdisk storage (must persist for the lifetime of the VFS)
// ------------------------------

template <typename IO>
using RamdiskT = Fat16<IO>;

static byte gRamdiskIoStorage[sizeof(io::InMemory)];
static byte gRamdiskFsStorage[sizeof(RamdiskT<io::InMemory>)];

// ------------------------------
// Construction
// ------------------------------

internal::VfsModule::VfsModule(const BootArguments &args) noexcept
{
    DEBUG_INFO_VFS("VfsModule::VfsModule()");

    // Mount ramdisk if available
    if constexpr (FeatureEnabled<FeatureFlag::kRamdisk>) {
        if (args.ramdisk_args.start != nullptr && args.ramdisk_args.end > args.ramdisk_args.start) {
            auto *ramdisk_virt = Mem::PhysToVirt(args.ramdisk_args.start);
            auto *io_ptr       = new (gRamdiskIoStorage) io::InMemory(ramdisk_virt);

            // Validate and create Fat12 filesystem
            if (RamdiskT<io::InMemory>::IsValid(*io_ptr)) {
                auto *fs_ptr = new (gRamdiskFsStorage) RamdiskT(*io_ptr);
                auto fs      = fs_ptr->GetFilesystem();

                // Mount at root
                auto mount_result = Mount(Path("/"), MountOptions{.read_only = false}, fs);
                if (!mount_result.has_value()) {
                    TRACE_FATAL_VFS("Failed to mount ramdisk");
                }
            } else {
                TRACE_FATAL_VFS("Ramdisk is not a valid FAT12 filesystem");
            }
        } else {
            TRACE_FATAL_VFS("No ramdisk available");
        }
    }
}

// ------------------------------
// Mount Point Management
// ------------------------------

Result<> internal::VfsModule::Mount(
    const Path &mount_path, MountOptions options, Filesystem &driver
)
{
    RET_UNEXPECTED_IF(mount_path.IsEmpty() || !mount_path.IsAbsolute(), VfsError::kInvalidPath);

    RET_UNEXPECTED_IF(GetMounts().Contains(mount_path.CString()), VfsError::kAlreadyMounted);

    bool inserted = GetMounts().Insert(mount_path.CString(), mount_path, options, driver);
    RET_UNEXPECTED_IF(!inserted, VfsError::kUnknownError);

    TRACE_INFO_VFS("Mounted %s filesystem at %s", driver.GetInfo().name, mount_path.CString());
    return {};
}

Result<> internal::VfsModule::Unmount(const Path &mount_path)
{
    RET_UNEXPECTED_IF(mount_path.IsEmpty() || !mount_path.IsAbsolute(), VfsError::kInvalidPath);

    RET_UNEXPECTED_IF(!GetMounts().Contains(mount_path.CString()), VfsError::kNotMounted);

    bool removed = GetMounts().Remove(mount_path.CString());
    RET_UNEXPECTED_IF(!removed, VfsError::kUnknownError);

    TRACE_INFO_VFS("Unmounted filesystem at %s", mount_path.CString());
    return {};
}

Result<MountPoint *> internal::VfsModule::FindMountPoint(const Path &path)
{
    RET_UNEXPECTED_IF(path.IsEmpty(), VfsError::kInvalidPath);

    auto match = GetMounts().GetLongestPrefixMatch(path.CString());
    RET_UNEXPECTED_IF(!match, VfsError::kMountPointNotFound);
    return *match;
}

// ------------------------------
// Path Utilities
// ------------------------------

Path internal::VfsModule::GetRelativePath_(const Path &absolute_path, const Path &mount_path)
{
    // If the mount point is root, the relative path is the same as absolute
    if (mount_path.IsRoot()) {
        return absolute_path;
    }

    // Build relative path by skipping mount_path components
    size_t mount_components    = mount_path.ComponentCount();
    size_t absolute_components = absolute_path.ComponentCount();

    if (mount_components >= absolute_components) {
        // The path is the mount point itself, return root
        return Path("/");
    }

    // Build path from remaining components
    Path result("/");
    for (size_t i = mount_components; i < absolute_components; ++i) {
        result /= absolute_path.GetComponent(i);
    }

    return result;
}

// ------------------------------
// File Operations
// ------------------------------

Result<> internal::VfsModule::CreateFile(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount = mount_result.value();

    RET_UNEXPECTED_IF(mount->options.read_only, VfsError::kReadOnly);

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.CreateFile(relative_path);
}

Result<size_t> internal::VfsModule::ReadFile(
    const Path &path, void *buffer, size_t size, size_t offset
)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.ReadFile(relative_path, buffer, size, offset);
}

Result<size_t> internal::VfsModule::WriteFile(
    const Path &path, const void *buffer, size_t size, size_t offset
)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount = mount_result.value();

    RET_UNEXPECTED_IF(mount->options.read_only, VfsError::kReadOnly);

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.WriteFile(relative_path, buffer, size, offset);
}

Result<> internal::VfsModule::DeleteFile(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount = mount_result.value();

    RET_UNEXPECTED_IF(mount->options.read_only, VfsError::kReadOnly);

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.DeleteFile(relative_path);
}

Result<bool> internal::VfsModule::FileExists(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.FileExists(relative_path);
}

Result<size_t> internal::VfsModule::GetFileSize(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.GetFileSize(relative_path);
}

// ------------------------------
// Directory Operations
// ------------------------------

Result<> internal::VfsModule::CreateDirectory(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount = mount_result.value();

    RET_UNEXPECTED_IF(mount->options.read_only, VfsError::kReadOnly);

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.CreateDirectory(relative_path);
}

Result<> internal::VfsModule::RemoveDirectory(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount = mount_result.value();

    RET_UNEXPECTED_IF(mount->options.read_only, VfsError::kReadOnly);

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.RemoveDirectory(relative_path);
}

Result<bool> internal::VfsModule::DirectoryExists(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.DirectoryExists(relative_path);
}

// ------------------------------
// General Operations
// ------------------------------

Result<bool> internal::VfsModule::Exists(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    RET_UNEXPECTED_IF_ERR(mount_result);

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.Exists(relative_path);
}

Result<> internal::VfsModule::Move(const Path &old_path, const Path &new_path)
{
    // Find mount points for both paths
    auto old_mount_result = FindMountPoint(old_path);
    RET_UNEXPECTED_IF_ERR(old_mount_result);

    auto new_mount_result = FindMountPoint(new_path);
    RET_UNEXPECTED_IF_ERR(new_mount_result);

    MountPoint *old_mount = old_mount_result.value();
    MountPoint *new_mount = new_mount_result.value();

    // Cross-filesystem move is not supported
    RET_UNEXPECTED_IF(old_mount != new_mount, VfsError::kCrossFilesystemRename);

    if (old_mount->options.read_only) {
        return std::unexpected(VfsError::kReadOnly);
    }

    Path old_relative = GetRelativePath_(old_path, old_mount->path);
    Path new_relative = GetRelativePath_(new_path, new_mount->path);

    return old_mount->fs.Move(old_relative, new_relative);
}
