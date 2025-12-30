#include "file_descriptor.hpp"
#include "modules/scheduling.hpp"

#include <string.h>
#include <mutex.hpp>
#include <vfs.hpp>

#include "scheduling/processes.hpp"

namespace Fs
{

// ============================================================================
// FdTable Implementation
// ============================================================================

FdResult<fd_t> FdTable::AllocateFd(OpenFileEntry *global_entry)
{
    ASSERT_NOT_NULL(global_entry);

    std::lock_guard lock(lock_);

    // Find the first available file descriptor
    for (size_t i = 0; i < kMaxFds; ++i) {
        if (!entries_[i].IsValid()) {
            entries_[i] = FdEntry::Wrap(global_entry);
            open_count_++;
            return static_cast<fd_t>(i);
        }
    }

    return std::unexpected(FdError::kFdTableFull);
}

FdResult<fd_t> FdTable::AllocateFdAt(OpenFileEntry *global_entry, fd_t fd)
{
    ASSERT_NOT_NULL(global_entry);

    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    // Check if the FD is already in use
    RET_UNEXPECTED_IF(entries_[fd].IsValid(), FdError::kBadFileDescriptor);

    // Allocate at the specific index
    entries_[fd] = FdEntry::Wrap(global_entry);
    open_count_++;

    return fd;
}

FdResult<> FdTable::FreeFd(fd_t fd)
{
    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    // Prevent closing standard streams through normal operations
    RET_UNEXPECTED_IF(
        fd <= kStderrFd && entries_[fd].Is<IO::Pipe<4096>>(), FdError::kPermissionDenied
    );

    RET_UNEXPECTED_IF(!entries_[fd].IsValid(), FdError::kNotOpen);

    entries_[fd] = {};
    open_count_--;

    return {};
}

Fs::FdTable::FdEntry *FdTable::GetEntry(fd_t fd)
{
    if (!IsValidFd(fd)) {
        return nullptr;
    }
    return &entries_[fd];
}

bool FdTable::IsValidFd(fd_t fd) const { return fd >= 0 && static_cast<size_t>(fd) < kMaxFds; }

FdResult<fd_t> FdTable::DuplicateFd(fd_t fd)
{
    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    RET_UNEXPECTED_IF(!entries_[fd].IsValid(), FdError::kBadFileDescriptor);

    // Find the first available file descriptor for the copy
    for (size_t i = 0; i < kMaxFds; ++i) {
        if (!entries_[i].IsValid()) {
            entries_[i] = std::move(entries_[fd]);
            open_count_++;

            return static_cast<fd_t>(i);
        }
    }

    return std::unexpected(FdError::kFdTableFull);
}

// ============================================================================
// FileTable Implementation
// ============================================================================

FdResult<File *> FileTable::GetOrCreate(const vfs::Path &path)
{
    RET_UNEXPECTED_IF(path.IsEmpty(), FdError::kInvalidArgument);

    std::lock_guard lock(lock_);

    // Check if file already exists
    auto existing = Find(path);
    if (existing.has_value()) {
        File *file = *existing;
        file->ref_count++;
        return file;
    }

    // Find a free file slot
    size_t free_slot = kMaxFiles;
    for (size_t i = 0; i < kMaxFiles; ++i) {
        if (files_[i].ref_count == 0) {
            free_slot = i;
            break;
        }
    }

    RET_UNEXPECTED_IF(free_slot == kMaxFiles, FdError::kIoError);

    // Initialize the new file
    File *file      = &files_[free_slot];
    file->size      = 0;  // TODO: Get from VFS when implemented
    file->mode      = 0;
    file->ref_count = 0;
    file->path      = std::move(path);

    count_++;
    return file;
}

FdResult<> FileTable::Release(File *file)
{
    if (file == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    RET_UNEXPECTED_IF(file->ref_count == 0, FdError::kNotOpen);

    file->ref_count--;

    if (file->ref_count == 0) {
        // TODO: Clean up file resources if needed
        count_--;
    }

    return {};
}

std::optional<File *> FileTable::Find(const vfs::Path &path)
{
    for (size_t i = 0; i < kMaxFiles; ++i) {
        if (files_[i].ref_count > 0 && files_[i].path == path) {
            std::optional<File *> result;
            result.emplace(&files_[i]);
            return result;
        }
    }

    return {};
}

// ============================================================================
// OpenFileTable Implementation
// ============================================================================

OpenFileTable::OpenFileTable() : open_count_(0)
{
    // Initialize all entries as unused
    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        entries_[i].flags     = 0;
        entries_[i].offset    = 0;
        entries_[i].ref_count = 0;
        entries_[i].file      = nullptr;
        entries_[i].is_append = false;
    }
}

OpenFileTable::~OpenFileTable()
{
    std::lock_guard lock(lock_);

    // All entries should be closed by now, but clean up just in case
    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        if (entries_[i].ref_count > 0) {
            // Decrement file reference count
            if (entries_[i].file != nullptr) {
                entries_[i].file->ref_count--;
            }
        }
    }
}

FdError OpenFileTable::OpenFile(File *file, OpenMode flags, OpenFileEntry **entry_out)
{
    if (file == nullptr || entry_out == nullptr) {
        return FdError::kInvalidArgument;
    }

    std::lock_guard lock(lock_);

    // Find a free entry slot
    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        if (entries_[i].ref_count == 0) {
            // Initialize the new entry
            entries_[i].flags     = static_cast<u32>(flags);
            entries_[i].offset    = 0;
            entries_[i].ref_count = 1;
            entries_[i].file      = file;
            entries_[i].is_append = HasMode(flags, OpenMode::kAppend);

            // Increment file reference count
            file->ref_count++;

            open_count_++;
            *entry_out = &entries_[i];
            return FdError{};  // Success (no error)
        }
    }

    return FdError::kIoError;  // No free entry slots
}

FdError OpenFileTable::CloseFile(OpenFileEntry *entry)
{
    if (entry == nullptr) {
        return FdError::kInvalidArgument;
    }

    std::lock_guard lock(lock_);

    if (entry->ref_count == 0) {
        return FdError::kAlreadyClosed;
    }

    entry->ref_count--;

    // If no more references, clean up entry
    if (entry->ref_count == 0) {
        if (entry->file != nullptr) {
            // Decrement file reference count
            entry->file->ref_count--;
            entry->file = nullptr;
        }
        open_count_--;
    }

    return FdError{};  // Success (no error)
}

FdError OpenFileTable::GetOffset(OpenFileEntry *entry, u64 *offset_out)
{
    if (entry == nullptr || offset_out == nullptr) {
        return FdError::kInvalidArgument;
    }

    std::lock_guard lock(lock_);

    *offset_out = entry->offset;
    return FdError{};  // Success (no error)
}

FdError OpenFileTable::SetOffset(OpenFileEntry *entry, u64 offset)
{
    if (entry == nullptr) {
        return FdError::kInvalidArgument;
    }

    std::lock_guard lock(lock_);

    entry->offset = offset;
    return FdError{};  // Success (no error)
}

// ============================================================================
// FdManager Implementation
// ============================================================================

FdManager::FdManager() {}

FdManager::~FdManager() {}

FdResult<fd_t> FdManager::Open(const vfs::Path &path, OpenMode flags)
{
    // Get or create file
    auto file_result = file_table_.GetOrCreate(path);
    RET_UNEXPECTED_IF_ERR(file_result);
    File *file = *file_result;

    // Create global file entry
    OpenFileEntry *global_entry = nullptr;
    FdError err                 = open_file_table_.OpenFile(file, flags, &global_entry);
    if (err != FdError{}) {
        // Clean up file reference
        file_table_.Release(file);
        return std::unexpected(err);
    }

    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    if (fd_table == nullptr) {
        // Clean up global entry
        open_file_table_.CloseFile(global_entry);
        return std::unexpected(FdError::kIoError);
    }

    // Allocate file descriptor
    auto fd_result = fd_table->AllocateFd(global_entry);
    if (!fd_result) {
        // Clean up global entry
        open_file_table_.CloseFile(global_entry);
        return std::unexpected(fd_result.error());
    }

    return *fd_result;
}

FdResult<> FdManager::Close(int fd)
{
    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    RET_UNEXPECTED_IF(fd_table == nullptr, FdError::kIoError);

    // Get entry using TaggedPointer
    auto *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry || !entry->IsValid(), FdError::kBadFileDescriptor);

    // Dispatch based on type - close OpenFileEntry if needed
    if (entry->Is<OpenFileEntry>()) {
        OpenFileEntry &global_entry = entry->As<OpenFileEntry>();

        // Close global entry
        FdError err = open_file_table_.CloseFile(&global_entry);
        if (err != FdError{}) {
            return std::unexpected(err);
        }
    }

    // Free file descriptor
    auto free_result = fd_table->FreeFd(fd);
    RET_UNEXPECTED_IF(!free_result, free_result.error());

    return {};  // Success
}

FdResult<size_t> FdManager::Read(int fd, std::span<byte> buffer)
{
    RET_UNEXPECTED_IF(buffer.empty(), FdError::kInvalidArgument);

    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    // Get entry using TaggedPointer
    auto *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    // Dispatch based on type using visitor pattern
    return entry->Visit([&]<typename T>(T &value) -> FdResult<size_t> {
        if constexpr (std::same_as<T, OpenFileEntry>) {
            // Check if file is opened for reading
            OpenMode mode = static_cast<OpenMode>(value.flags);
            RET_UNEXPECTED_IF(!HasMode(mode, OpenMode::kRead), FdError::kPermissionDenied);

            // Get file
            File *file = value.file;
            RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

            // Read from file
            auto result = vfs::ReadFile(file->path, buffer.data(), buffer.size(), value.offset);
            RET_UNEXPECTED_IF(!result, FdError::kIoError);

            // Update offset
            value.offset += *result;
            return *result;
        } else {
            auto result = value.Read(buffer);
            RET_UNEXPECTED_IF(!result, FdError::kIoError);
            return *result;
        }
    });
}

FdResult<size_t> FdManager::Write(int fd, std::span<const byte> buffer)
{
    RET_UNEXPECTED_IF(buffer.empty(), FdError::kInvalidArgument);

    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    // Get entry using TaggedPointer
    auto *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry || !entry->IsValid(), FdError::kBadFileDescriptor);

    // Dispatch based on type using visitor pattern
    return entry->Visit([&]<typename T>(T &value) -> FdResult<size_t> {
        if constexpr (std::is_same_v<T, OpenFileEntry>) {
            // Check if file is opened for writing
            OpenMode mode = static_cast<OpenMode>(value.flags);
            RET_UNEXPECTED_IF(!HasMode(mode, OpenMode::kWrite), FdError::kPermissionDenied);

            // Get file
            File *file = value.file;
            RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

            // For append mode, seek to end of file
            if (value.is_append) {
                value.offset = file->size;
            }

            // Write to stream
            auto result = vfs::WriteFile(file->path, buffer.data(), buffer.size(), value.offset);
            RET_UNEXPECTED_IF(!result, FdError::kIoError);

            // Update offset
            value.offset += *result;
            return *result;
        } else {
            // Standard stream handling (IO::Pipe<4096>)
            auto result = value.Write(buffer);
            RET_UNEXPECTED_IF(!result, FdError::kIoError);
            return *result;
        }
    });
}

FdResult<ssize_t> FdManager::Seek(fd_t fd, ssize_t offset, FdSeek whence)
{
    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    // Get entry using TaggedPointer
    auto *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    // Dispatch based on type using visitor pattern
    return entry->Visit([&]<typename T>(T &value) -> FdResult<ssize_t> {
        if constexpr (std::is_same_v<T, OpenFileEntry>) {
            // Get file
            File *file = value.file;
            RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

            // Determine new offset based on whence
            ssize_t new_offset = 0;
            switch (whence) {
                case FdSeek::kSet:
                    new_offset = offset;
                    break;
                case FdSeek::kCur:
                    new_offset = value.offset + offset;
                    break;
                case FdSeek::kEnd:
                    new_offset = file->size + offset;
                    break;
                default:
                    return std::unexpected(FdError::kInvalidArgument);
            }

            RET_UNEXPECTED_IF(
                new_offset > static_cast<ssize_t>(file->size) || new_offset < 0,
                FdError::kInvalidArgument
            );

            value.offset = new_offset;
            return new_offset;
        } else {
            return std::unexpected(FdError::kInvalidArgument);
        }
    });
}

FdTable *FdManager::GetCurrentProcessFdTable()
{
    auto process = SchedulingModule::Get().GetProcesses().GetCurrentProcess();
    if (!process) {
        return nullptr;
    }

    return process.value()->fd_table;
}

}  // namespace Fs
