#include "vfs.hpp"
#include "../vfs.hpp"

#include <autogen/feature_flags.h>
#include <mem/heap.hpp>
#include <mem/types.hpp>
#include <trace_framework.hpp>
#include <vfs/fat/fat12.hpp>
#include <vfs/io/in_memory.hpp>
#include "boot_args.hpp"

using namespace vfs;

// ------------------------------
// Global ramdisk storage (must persist for the lifetime of the VFS)
// ------------------------------

static byte gRamdiskIoStorage[sizeof(io::InMemory)];
static byte gRamdiskFsStorage[sizeof(Fat12<io::InMemory>)];

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
            if (Fat12<io::InMemory>::IsValid(*io_ptr)) {
                auto *fat12_ptr = new (gRamdiskFsStorage) Fat12(*io_ptr);
                auto fs         = fat12_ptr->GetFilesystem();

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
    if (mount_path.IsEmpty() || !mount_path.IsAbsolute()) {
        return std::unexpected(VfsError::kInvalidPath);
    }

    if (GetMounts().Contains(mount_path.CString())) {
        return std::unexpected(VfsError::kAlreadyMounted);
    }

    bool inserted = GetMounts().Insert(mount_path.CString(), mount_path, options, driver);
    if (!inserted) {
        return std::unexpected(VfsError::kUnknownError);
    }

    TRACE_INFO_VFS("Mounted %s filesystem at %s", driver.GetInfo().name, mount_path.CString());
    return {};
}

Result<> internal::VfsModule::Unmount(const Path &mount_path)
{
    if (mount_path.IsEmpty() || !mount_path.IsAbsolute()) {
        return std::unexpected(VfsError::kInvalidPath);
    }

    if (!GetMounts().Contains(mount_path.CString())) {
        return std::unexpected(VfsError::kNotMounted);
    }

    bool removed = GetMounts().Remove(mount_path.CString());
    if (!removed) {
        return std::unexpected(VfsError::kUnknownError);
    }

    TRACE_INFO_VFS("Unmounted filesystem at %s", mount_path.CString());
    return {};
}

Result<MountPoint *> internal::VfsModule::FindMountPoint(const Path &path)
{
    if (path.IsEmpty()) {
        return std::unexpected(VfsError::kInvalidPath);
    }

    auto match = GetMounts().GetLongestPrefixMatch(path.CString());
    if (match) {
        return *match;
    }

    return std::unexpected(VfsError::kMountPointNotFound);
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
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount = mount_result.value();

    if (mount->options.read_only) {
        return std::unexpected(VfsError::kReadOnly);
    }

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.CreateFile(relative_path);
}

Result<size_t> internal::VfsModule::ReadFile(
    const Path &path, void *buffer, size_t size, size_t offset
)
{
    auto mount_result = FindMountPoint(path);
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.ReadFile(relative_path, buffer, size, offset);
}

Result<size_t> internal::VfsModule::WriteFile(
    const Path &path, const void *buffer, size_t size, size_t offset
)
{
    auto mount_result = FindMountPoint(path);
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount = mount_result.value();

    if (mount->options.read_only) {
        return std::unexpected(VfsError::kReadOnly);
    }

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.WriteFile(relative_path, buffer, size, offset);
}

Result<> internal::VfsModule::DeleteFile(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount = mount_result.value();

    if (mount->options.read_only) {
        return std::unexpected(VfsError::kReadOnly);
    }

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.DeleteFile(relative_path);
}

Result<bool> internal::VfsModule::FileExists(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.FileExists(relative_path);
}

Result<size_t> internal::VfsModule::GetFileSize(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

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
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount = mount_result.value();

    if (mount->options.read_only) {
        return std::unexpected(VfsError::kReadOnly);
    }

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.CreateDirectory(relative_path);
}

Result<> internal::VfsModule::RemoveDirectory(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount = mount_result.value();

    if (mount->options.read_only) {
        return std::unexpected(VfsError::kReadOnly);
    }

    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.RemoveDirectory(relative_path);
}

Result<bool> internal::VfsModule::DirectoryExists(const Path &path)
{
    auto mount_result = FindMountPoint(path);
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

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
    if (!mount_result.has_value()) {
        return std::unexpected(mount_result.error());
    }

    MountPoint *mount  = mount_result.value();
    Path relative_path = GetRelativePath_(path, mount->path);
    return mount->fs.Exists(relative_path);
}

Result<> internal::VfsModule::Move(const Path &old_path, const Path &new_path)
{
    // Find mount points for both paths
    auto old_mount_result = FindMountPoint(old_path);
    if (!old_mount_result.has_value()) {
        return std::unexpected(old_mount_result.error());
    }

    auto new_mount_result = FindMountPoint(new_path);
    if (!new_mount_result.has_value()) {
        return std::unexpected(new_mount_result.error());
    }

    MountPoint *old_mount = old_mount_result.value();
    MountPoint *new_mount = new_mount_result.value();

    // Cross-filesystem move is not supported
    if (old_mount != new_mount) {
        return std::unexpected(VfsError::kCrossFilesystemRename);
    }

    if (old_mount->options.read_only) {
        return std::unexpected(VfsError::kReadOnly);
    }

    Path old_relative = GetRelativePath_(old_path, old_mount->path);
    Path new_relative = GetRelativePath_(new_path, new_mount->path);

    return old_mount->fs.Move(old_relative, new_relative);
}
