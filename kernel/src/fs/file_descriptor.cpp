#include "file_descriptor.hpp"
#include "modules/scheduling.hpp"

#include <mutex.hpp>
#include <vfs.hpp>

#include "scheduling/processes.hpp"

namespace
{

bool IsValidFd(fd_t fd) { return fd >= 0 && static_cast<size_t>(fd) < Fs::kMaxFdsPerProcess; }

}  // namespace

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
    for (size_t i = 0; i < kMaxFdsPerProcess; ++i) {
        if (entries_[i] == nullptr) {
            entries_[i] = global_entry;
            global_entry->ref_count++;
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
    RET_UNEXPECTED_IF(entries_[fd] != nullptr, FdError::kBadFileDescriptor);

    // Allocate at the specific index
    entries_[fd] = global_entry;
    global_entry->ref_count++;
    open_count_++;

    return fd;
}

FdResult<> FdTable::FreeFd(fd_t fd)
{
    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    // Prevent closing standard streams through normal operations
    if (fd <= kStderrFd && entries_[fd] != nullptr && entries_[fd]->IsPipe()) {
        return std::unexpected(FdError::kPermissionDenied);
    }

    RET_UNEXPECTED_IF(entries_[fd] == nullptr, FdError::kNotOpen);

    entries_[fd] = nullptr;
    open_count_--;

    return {};
}

OpenFileEntry *FdTable::GetEntry(fd_t fd)
{
    if (!IsValidFd(fd)) {
        return nullptr;
    }
    return entries_[fd];
}

const OpenFileEntry *FdTable::GetEntry(fd_t fd) const
{
    if (!IsValidFd(fd)) {
        return nullptr;
    }
    return entries_[fd];
}

FdResult<fd_t> FdTable::DuplicateFd(fd_t fd)
{
    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    RET_UNEXPECTED_IF(entries_[fd] == nullptr, FdError::kBadFileDescriptor);

    // Find the first available file descriptor for the copy
    for (size_t i = 0; i < kMaxFdsPerProcess; ++i) {
        if (entries_[i] == nullptr) {
            // Copy the pointer and increment ref count
            entries_[i] = entries_[fd];
            entries_[i]->ref_count++;
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

    // Check if file already exists (under lock, safe to return pointer)
    File *existing = const_cast<File *>(Find(path));
    if (existing != nullptr) {
        existing->ref_count++;
        return existing;
    }

    // Find a free file slot
    size_t free_slot = kMaxActiveFiles;
    for (size_t i = 0; i < kMaxActiveFiles; ++i) {
        if (files_[i].ref_count == 0) {
            free_slot = i;
            break;
        }
    }

    RET_UNEXPECTED_IF(free_slot == kMaxActiveFiles, FdError::kIoError);

    // Initialize the new file
    File *file      = &files_[free_slot];
    file->size      = 0;  // TODO: Get from VFS when implemented
    file->mode      = 0;
    file->ref_count = 1;  // Start with ref_count = 1
    file->path      = path;

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

File *FileTable::Find(const vfs::Path &path)
{
    for (size_t i = 0; i < kMaxActiveFiles; ++i) {
        if (files_[i].ref_count > 0 && files_[i].path == path) {
            return &files_[i];
        }
    }
    return nullptr;
}

const File *FileTable::Find(const vfs::Path &path) const
{
    for (size_t i = 0; i < kMaxActiveFiles; ++i) {
        if (files_[i].ref_count > 0 && files_[i].path == path) {
            return &files_[i];
        }
    }
    return nullptr;
}

// ============================================================================
// OpenFileTable Implementation
// ============================================================================

OpenFileTable::OpenFileTable() : open_count_(0)
{
    // Initialize all entries as unused
    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        entries_[i].handle    = {};
        entries_[i].flags     = 0;
        entries_[i].offset    = 0;
        entries_[i].ref_count = 0;
        entries_[i].is_append = false;
    }
}

OpenFileTable::~OpenFileTable()
{
    std::lock_guard lock(lock_);

    // All entries should be closed by now, but clean up just in case
    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        if (entries_[i].ref_count > 0) {
            if (entries_[i].IsFile() && entries_[i].GetFile() != nullptr) {
                // Decrement file reference count
                entries_[i].GetFile()->ref_count--;
            }
            // Pipes are not owned by OpenFileEntry, they're owned by Process
        }
    }
}

FdResult<OpenFileEntry *> OpenFileTable::OpenFile(File *file, OpenMode flags)
{
    if (file == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    // Find a free entry slot
    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        if (entries_[i].ref_count == 0) {
            // Initialize the new entry with File handle (non-owned)
            entries_[i].handle    = FileHandle::Wrap(file);
            entries_[i].flags     = static_cast<u32>(flags);
            entries_[i].offset    = 0;
            entries_[i].ref_count = 1;
            entries_[i].is_append = HasMode(flags, OpenMode::kAppend);

            // Increment file reference count
            file->ref_count++;

            open_count_++;
            return &entries_[i];
        }
    }

    return std::unexpected(FdError::kIoError);
}

FdResult<OpenFileEntry *> OpenFileTable::OpenPipe(IO::Pipe<kStdioBufferSize> &pipe)
{
    std::lock_guard lock(lock_);

    // Find a free entry slot
    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        if (entries_[i].ref_count == 0) {
            // Initialize the new entry with Pipe handle (non-owned reference)
            entries_[i].handle    = FileHandle::Wrap(&pipe);
            entries_[i].flags     = static_cast<u32>(OpenMode::kReadWrite);
            entries_[i].offset    = 0;
            entries_[i].ref_count = 1;
            entries_[i].is_append = false;

            open_count_++;
            return &entries_[i];
        }
    }

    return std::unexpected(FdError::kIoError);
}

FdResult<> OpenFileTable::CloseFile(OpenFileEntry *entry)
{
    if (entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    if (entry->ref_count == 0) {
        return std::unexpected(FdError::kAlreadyClosed);
    }

    entry->ref_count--;

    // If no more references, clean up entry
    if (entry->ref_count == 0) {
        if (entry->IsFile() && entry->GetFile() != nullptr) {
            // Decrement file reference count
            entry->GetFile()->ref_count--;
        }
        // Pipes are not owned, just clear the handle reference
        entry->handle = {};
        open_count_--;
    }

    return {};
}

FdResult<u64> OpenFileTable::GetOffset(const OpenFileEntry *entry) const
{
    if (entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    return entry->offset;
}

FdResult<> OpenFileTable::SetOffset(OpenFileEntry *entry, u64 offset) const
{
    if (entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    entry->offset = offset;
    return {};
}

// ============================================================================
// FdManager Implementation
// ============================================================================

FdManager::FdManager() = default;

FdManager::~FdManager() = default;

FdResult<fd_t> FdManager::Open(const vfs::Path &path, OpenMode flags)
{
    // Get or create file (Lock level 1: FileTable)
    auto file_result = file_table_.GetOrCreate(path);
    RET_UNEXPECTED_IF_ERR(file_result);
    File *file = *file_result;

    // Create global file entry (Lock level 2: OpenFileTable)
    auto open_result = open_file_table_.OpenFile(file, flags);
    if (!open_result) {
        // Clean up file reference on failure
        file_table_.Release(file);
        return std::unexpected(open_result.error());
    }
    OpenFileEntry *global_entry = *open_result;

    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    if (fd_table == nullptr) {
        // Clean up global entry on failure
        open_file_table_.CloseFile(global_entry);
        return std::unexpected(FdError::kIoError);
    }

    // Allocate file descriptor (Lock level 3: FdTable)
    auto fd_result = fd_table->AllocateFd(global_entry);
    if (!fd_result) {
        // Clean up global entry on failure
        open_file_table_.CloseFile(global_entry);
        return std::unexpected(fd_result.error());
    }

    return *fd_result;
}

FdResult<> FdManager::Close(fd_t fd)
{
    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    RET_UNEXPECTED_IF(fd_table == nullptr, FdError::kIoError);

    // Get entry
    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    // Close global entry (Lock level 2: OpenFileTable)
    auto close_result = open_file_table_.CloseFile(entry);
    RET_UNEXPECTED_IF(!close_result, close_result.error());

    // Free file descriptor (Lock level 3: FdTable)
    auto free_result = fd_table->FreeFd(fd);
    RET_UNEXPECTED_IF(!free_result, free_result.error());

    return {};
}

FdResult<size_t> FdManager::Read(fd_t fd, std::span<byte> buffer)
{
    RET_UNEXPECTED_IF(buffer.empty(), FdError::kInvalidArgument);

    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    // Get entry
    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    // Check if file is opened for reading
    OpenMode mode = static_cast<OpenMode>(entry->flags);
    RET_UNEXPECTED_IF(!HasMode(mode, OpenMode::kRead), FdError::kPermissionDenied);

    // Dispatch based on handle type
    if (entry->IsFile()) {
        File *file = entry->GetFile();
        RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

        // Read from file
        auto result = vfs::ReadFile(file->path, buffer.data(), buffer.size(), entry->offset);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);

        // Update offset
        entry->offset += *result;
        return *result;
    } else if (entry->IsPipe()) {
        auto *pipe = entry->GetPipe();
        RET_UNEXPECTED_IF(pipe == nullptr, FdError::kBadFileDescriptor);

        auto result = pipe->Read(buffer);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);
        return *result;
    }

    return std::unexpected(FdError::kBadFileDescriptor);
}

FdResult<size_t> FdManager::Write(fd_t fd, std::span<const byte> buffer)
{
    RET_UNEXPECTED_IF(buffer.empty(), FdError::kInvalidArgument);

    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    // Get entry
    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    // Check if file is opened for writing
    OpenMode mode = static_cast<OpenMode>(entry->flags);
    RET_UNEXPECTED_IF(!HasMode(mode, OpenMode::kWrite), FdError::kPermissionDenied);

    // Dispatch based on handle type
    if (entry->IsFile()) {
        File *file = entry->GetFile();
        RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

        // For append mode, seek to end of file
        if (entry->is_append) {
            entry->offset = file->size;
        }

        // Write to stream
        auto result = vfs::WriteFile(file->path, buffer.data(), buffer.size(), entry->offset);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);

        // Update offset
        entry->offset += *result;
        return *result;
    } else if (entry->IsPipe()) {
        auto *pipe = entry->GetPipe();
        RET_UNEXPECTED_IF(pipe == nullptr, FdError::kBadFileDescriptor);

        auto result = pipe->Write(buffer);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);
        return *result;
    }

    return std::unexpected(FdError::kBadFileDescriptor);
}

FdResult<ssize_t> FdManager::Seek(fd_t fd, ssize_t offset, FdSeek whence)
{
    // Get current process FD table
    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    // Get entry
    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    // Only files support seeking, not pipes
    RET_UNEXPECTED_IF(!entry->IsFile(), FdError::kInvalidArgument);

    File *file = entry->GetFile();
    RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

    // Determine new offset based on whence
    ssize_t new_offset = 0;
    switch (whence) {
        case FdSeek::kSet:
            new_offset = offset;
            break;
        case FdSeek::kCur:
            new_offset = entry->offset + offset;
            break;
        case FdSeek::kEnd:
            new_offset = file->size + offset;
            break;
        default:
            return std::unexpected(FdError::kInvalidArgument);
    }

    RET_UNEXPECTED_IF(
        new_offset > static_cast<ssize_t>(file->size) || new_offset < 0, FdError::kInvalidArgument
    );

    entry->offset = static_cast<u64>(new_offset);
    return new_offset;
}

FdTable *FdManager::GetCurrentProcessFdTable()
{
    auto process = SchedulingModule::Get().GetProcesses().GetCurrentProcess();
    if (!process) {
        return nullptr;
    }

    return process.value()->fd_table;
}

const FdTable *FdManager::GetCurrentProcessFdTable() const
{
    auto process = SchedulingModule::Get().GetProcesses().GetCurrentProcess();
    if (!process) {
        return nullptr;
    }

    return process.value()->fd_table;
}

}  // namespace Fs
