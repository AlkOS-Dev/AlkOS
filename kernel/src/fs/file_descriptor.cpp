#include "file_descriptor.hpp"
#include "modules/scheduling.hpp"
#include "vfs_stream.hpp"

#include <string.h>

#include "hal/debug_terminal.hpp"
#include "hal/panic.hpp"
#include "hal/spinlock.hpp"
#include "io/error.hpp"
#include "scheduling/processes.hpp"

namespace Fs
{

// ============================================================================
// Constants and Helpers
// ============================================================================

namespace
{
// Default file permissions
constexpr u32 kDefaultFileMode = 0644;

/**
 * @brief RAII lock guard for spinlock
 */
class LockGuard
{
    public:
    explicit LockGuard(arch::Spinlock &lock) : lock_(lock) { lock_.Lock(); }

    ~LockGuard() { lock_.Unlock(); }

    // Non-copyable, non-movable
    LockGuard(const LockGuard &)            = delete;
    LockGuard &operator=(const LockGuard &) = delete;
    LockGuard(LockGuard &&)                 = delete;
    LockGuard &operator=(LockGuard &&)      = delete;

    private:
    arch::Spinlock &lock_;
};

}  // namespace

// ============================================================================
// FdTable Implementation
// ============================================================================

FdTable::FdTable() : open_count_(0), standard_streams_initialized_(false)
{
    // Initialize all entries as unused
    for (size_t i = 0; i < kMaxFds; ++i) {
        entries_[i].global_entry       = nullptr;
        entries_[i].fd_flags           = 0;
        entries_[i].is_standard_stream = false;
    }
}

FdTable::~FdTable()
{
    // Close all open file descriptors
    LockGuard lock(lock_);

    for (size_t i = 0; i < kMaxFds; ++i) {
        if (entries_[i].global_entry != nullptr) {
            // Note: We cannot close global entries here as we don't have access
            // to the OpenFileTable. This cleanup should be handled by FdManager.
            entries_[i].global_entry = nullptr;
        }
    }
}

FdResult<fd_t> FdTable::AllocateFd(OpenFileEntry *global_entry)
{
    if (global_entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    LockGuard lock(lock_);

    // Find the first available file descriptor
    for (size_t i = 0; i < kMaxFds; ++i) {
        if (entries_[i].global_entry == nullptr) {
            entries_[i].global_entry       = global_entry;
            entries_[i].fd_flags           = 0;
            entries_[i].is_standard_stream = false;
            open_count_++;
            return static_cast<fd_t>(i);
        }
    }

    return std::unexpected(FdError::kFdTableFull);
}

FdResult<fd_t> FdTable::AllocateFdAt(OpenFileEntry *global_entry, fd_t fd)
{
    if (global_entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    if (!IsValidFd(fd)) {
        return std::unexpected(FdError::kInvalidFd);
    }

    LockGuard lock(lock_);

    // Check if the FD is already in use
    if (entries_[fd].global_entry != nullptr) {
        return std::unexpected(FdError::kFdTableFull);
    }

    // Allocate at the specific index
    entries_[fd].global_entry       = global_entry;
    entries_[fd].fd_flags           = 0;
    entries_[fd].is_standard_stream = (fd <= kStderrFd);
    open_count_++;

    return fd;
}

FdResult<> FdTable::FreeFd(fd_t fd)
{
    if (!IsValidFd(fd)) {
        return std::unexpected(FdError::kInvalidFd);
    }

    LockGuard lock(lock_);

    // Prevent closing standard streams through normal operations
    if (entries_[fd].is_standard_stream) {
        return std::unexpected(FdError::kPermissionDenied);
    }

    if (entries_[fd].global_entry == nullptr) {
        return std::unexpected(FdError::kNotOpen);
    }

    entries_[fd].global_entry       = nullptr;
    entries_[fd].fd_flags           = 0;
    entries_[fd].is_standard_stream = false;
    open_count_--;

    return {};
}

FdResult<OpenFileEntry *> FdTable::GetGlobalEntry(fd_t fd)
{
    if (!IsValidFd(fd)) {
        return std::unexpected(FdError::kInvalidFd);
    }

    LockGuard lock(lock_);

    OpenFileEntry *entry = entries_[fd].global_entry;
    if (entry == nullptr) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    return entry;
}

bool FdTable::IsValidFd(fd_t fd) const { return fd >= 0 && static_cast<size_t>(fd) < kMaxFds; }

FdResult<fd_t> FdTable::DuplicateFd(fd_t fd)
{
    if (!IsValidFd(fd)) {
        return std::unexpected(FdError::kInvalidFd);
    }

    LockGuard lock(lock_);

    if (entries_[fd].global_entry == nullptr) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    // Find the first available file descriptor for the copy
    for (size_t i = 0; i < kMaxFds; ++i) {
        if (entries_[i].global_entry == nullptr) {
            // Copy the entry
            entries_[i].global_entry       = entries_[fd].global_entry;
            entries_[i].fd_flags           = entries_[fd].fd_flags;
            entries_[i].is_standard_stream = false;
            open_count_++;

            return static_cast<fd_t>(i);
        }
    }

    return std::unexpected(FdError::kFdTableFull);
}

bool FdTable::IsStandardStreamInitialized() const
{
    LockGuard lock(lock_);
    return standard_streams_initialized_;
}

void FdTable::SetStandardStreamsInitialized(bool initialized)
{
    LockGuard lock(lock_);
    standard_streams_initialized_ = initialized;
}

// ============================================================================
// FileTable Implementation
// ============================================================================

FileTable::FileTable() : count_(0)
{
    // Initialize all files as unused
    for (size_t i = 0; i < kMaxFiles; ++i) {
        files_[i].size               = 0;
        files_[i].mode               = 0;
        files_[i].ref_count          = 0;
        files_[i].stream             = nullptr;
        files_[i].is_standard_stream = false;
    }
}

FileTable::~FileTable()
{
    LockGuard lock(lock_);

    // Clean up any remaining files
    for (size_t i = 0; i < kMaxFiles; ++i) {
        if (files_[i].ref_count > 0 && files_[i].stream != nullptr) {
            delete files_[i].stream;
            files_[i].stream = nullptr;
        }
    }
}

FdResult<File *> FileTable::GetOrCreate(const vfs::Path &path)
{
    if (path.IsEmpty()) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    LockGuard lock(lock_);

    // Check if file already exists
    auto existing = FindUnlocked(path);
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

    if (free_slot == kMaxFiles) {
        return std::unexpected(FdError::kIoError);  // No free file slots
    }

    // Initialize the new file
    File *file               = &files_[free_slot];
    file->size               = 0;  // TODO: Get from VFS when implemented
    file->mode               = kDefaultFileMode;
    file->ref_count          = 1;
    file->mount_point        = nullptr;
    file->stream             = new VfsStream(path.CString());
    file->is_standard_stream = false;

    count_++;
    return file;
}

FdResult<File *> FileTable::CreateStandardStream(StandardStreamType type)
{
    LockGuard lock(lock_);

    // Find a free file slot
    size_t free_slot = kMaxFiles;
    for (size_t i = 0; i < kMaxFiles; ++i) {
        if (files_[i].ref_count == 0) {
            free_slot = i;
            break;
        }
    }

    if (free_slot == kMaxFiles) {
        return std::unexpected(FdError::kIoError);  // No free file slots
    }

    // Initialize the new standard stream file
    File *file               = &files_[free_slot];
    file->size               = 0;
    file->mode               = kDefaultFileMode;
    file->ref_count          = 1;
    file->mount_point        = nullptr;  // Standard streams don't have mount points
    file->is_standard_stream = true;

    // Create appropriate stream wrapper based on type
    switch (type) {
        case StandardStreamType::kStdin:
            file->stream = new StdinStreamWrapper();
            break;
        case StandardStreamType::kStdout:
            file->stream = new StdoutStreamWrapper();
            break;
        case StandardStreamType::kStderr:
            file->stream = new StderrStreamWrapper();
            break;
    }

    count_++;
    return file;
}

FdResult<> FileTable::Release(File *file)
{
    if (file == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    LockGuard lock(lock_);

    if (file->ref_count == 0) {
        return std::unexpected(FdError::kNotOpen);
    }

    file->ref_count--;

    // If no more references, clean up file
    if (file->ref_count == 0) {
        if (file->stream != nullptr) {
            delete file->stream;
            file->stream = nullptr;
        }
        count_--;
    }

    return {};
}

std::optional<File *> FileTable::Find(const vfs::Path &path)
{
    if (path.IsEmpty()) {
        return {};
    }

    LockGuard lock(lock_);
    return FindUnlocked(path);
}

std::optional<File *> FileTable::FindUnlocked(const vfs::Path &path) const
{
    const char *path_cstr = path.CString();

    for (size_t i = 0; i < kMaxFiles; ++i) {
        if (files_[i].ref_count > 0 && !files_[i].is_standard_stream &&
            files_[i].stream != nullptr) {
            auto *vfs_stream = static_cast<VfsStream *>(files_[i].stream);
            if (strcmp(vfs_stream->GetPath(), path_cstr) == 0) {
                std::optional<File *> res;
                res.emplace(const_cast<File *>(&files_[i]));
                return res;
            }
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
    LockGuard lock(lock_);

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

    LockGuard lock(lock_);

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

    LockGuard lock(lock_);

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

    LockGuard lock(lock_);

    *offset_out = entry->offset;
    return FdError{};  // Success (no error)
}

FdError OpenFileTable::SetOffset(OpenFileEntry *entry, u64 offset)
{
    if (entry == nullptr) {
        return FdError::kInvalidArgument;
    }

    LockGuard lock(lock_);

    entry->offset = offset;
    return FdError{};  // Success (no error)
}

// ============================================================================
// FdManager Implementation
// ============================================================================

FdManager::FdManager() : standard_streams_initialized_(false) {}

FdManager::~FdManager() {}

FdResult<> FdManager::InitializeStandardStreams(FdTable &fd_table)
{
    // Check if already initialized
    if (standard_streams_initialized_) {
        return {};  // Already initialized, no error
    }

    // Use a scope guard to handle rollback on failure
    struct StandardStreamCleanup {
        File *stdin_file               = nullptr;
        File *stdout_file              = nullptr;
        File *stderr_file              = nullptr;
        OpenFileEntry *stdin_entry     = nullptr;
        OpenFileEntry *stdout_entry    = nullptr;
        OpenFileEntry *stderr_entry    = nullptr;
        bool stdin_fd_allocated        = false;
        bool stdout_fd_allocated       = false;
        bool stderr_fd_allocated       = false;
        FileTable *file_table          = nullptr;
        OpenFileTable *open_file_table = nullptr;
        FdTable::FdEntry *fd_entries   = nullptr;

        void Cleanup()
        {
            // Clean up in reverse order of allocation
            if (stderr_fd_allocated) {
                fd_entries[kStderrFd].global_entry       = nullptr;
                fd_entries[kStderrFd].fd_flags           = 0;
                fd_entries[kStderrFd].is_standard_stream = false;
            }
            if (stdout_fd_allocated) {
                fd_entries[kStdoutFd].global_entry       = nullptr;
                fd_entries[kStdoutFd].fd_flags           = 0;
                fd_entries[kStdoutFd].is_standard_stream = false;
            }
            if (stdin_fd_allocated) {
                fd_entries[kStdinFd].global_entry       = nullptr;
                fd_entries[kStdinFd].fd_flags           = 0;
                fd_entries[kStdinFd].is_standard_stream = false;
            }
            if (stderr_entry != nullptr) {
                open_file_table->CloseFile(stderr_entry);
            }
            if (stdout_entry != nullptr) {
                open_file_table->CloseFile(stdout_entry);
            }
            if (stdin_entry != nullptr) {
                open_file_table->CloseFile(stdin_entry);
            }
            if (stderr_file != nullptr) {
                file_table->Release(stderr_file);
            }
            if (stdout_file != nullptr) {
                file_table->Release(stdout_file);
            }
            if (stdin_file != nullptr) {
                file_table->Release(stdin_file);
            }
        }
    };

    StandardStreamCleanup cleanup;
    cleanup.file_table      = &file_table_;
    cleanup.open_file_table = &open_file_table_;
    cleanup.fd_entries      = fd_table.GetEntries();

    // Create standard stream files directly without using paths
    auto result = file_table_.CreateStandardStream(StandardStreamType::kStdin);
    if (!result) {
        return std::unexpected(result.error());
    }
    cleanup.stdin_file = *result;

    result = file_table_.CreateStandardStream(StandardStreamType::kStdout);
    if (!result) {
        cleanup.Cleanup();
        return std::unexpected(result.error());
    }
    cleanup.stdout_file = *result;

    result = file_table_.CreateStandardStream(StandardStreamType::kStderr);
    if (!result) {
        cleanup.Cleanup();
        return std::unexpected(result.error());
    }
    cleanup.stderr_file = *result;

    // Create global file entries
    FdError err =
        open_file_table_.OpenFile(cleanup.stdin_file, OpenMode::kRead, &cleanup.stdin_entry);
    if (err != FdError{}) {
        cleanup.Cleanup();
        return std::unexpected(err);
    }

    err = open_file_table_.OpenFile(cleanup.stdout_file, OpenMode::kWrite, &cleanup.stdout_entry);
    if (err != FdError{}) {
        cleanup.Cleanup();
        return std::unexpected(err);
    }

    err = open_file_table_.OpenFile(cleanup.stderr_file, OpenMode::kWrite, &cleanup.stderr_entry);
    if (err != FdError{}) {
        cleanup.Cleanup();
        return std::unexpected(err);
    }

    // Allocate file descriptors at explicit indices 0, 1, 2
    auto fd_result = fd_table.AllocateFdAt(cleanup.stdin_entry, kStdinFd);
    if (!fd_result) {
        cleanup.Cleanup();
        return std::unexpected(fd_result.error());
    }
    cleanup.stdin_fd_allocated = true;

    fd_result = fd_table.AllocateFdAt(cleanup.stdout_entry, kStdoutFd);
    if (!fd_result) {
        cleanup.Cleanup();
        return std::unexpected(fd_result.error());
    }
    cleanup.stdout_fd_allocated = true;

    fd_result = fd_table.AllocateFdAt(cleanup.stderr_entry, kStderrFd);
    if (!fd_result) {
        cleanup.Cleanup();
        return std::unexpected(fd_result.error());
    }
    cleanup.stderr_fd_allocated = true;

    // Mark as initialized
    standard_streams_initialized_ = true;
    fd_table.SetStandardStreamsInitialized(true);

    return {};  // Success
}

FdResult<fd_t> FdManager::Open(const vfs::Path &path, OpenMode flags)
{
    // Get or create file
    auto file_result = file_table_.GetOrCreate(path);
    if (!file_result) {
        return std::unexpected(file_result.error());
    }
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
    FdTable *fd_table = this->GetCurrentProcessFdTable();
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
    FdTable *fd_table = this->GetCurrentProcessFdTable();
    if (fd_table == nullptr) {
        return std::unexpected(FdError::kIoError);
    }

    // Get global entry
    auto entry_result = fd_table->GetGlobalEntry(fd);
    if (!entry_result) {
        return std::unexpected(entry_result.error());
    }
    OpenFileEntry *global_entry = *entry_result;

    // Close global entry
    FdError err = open_file_table_.CloseFile(global_entry);
    if (err != FdError{}) {
        return std::unexpected(err);
    }

    // Free file descriptor
    auto free_result = fd_table->FreeFd(fd);
    if (!free_result) {
        return std::unexpected(free_result.error());
    }

    return {};  // Success
}

FdResult<size_t> FdManager::Read(int fd, std::span<byte> buffer)
{
    if (buffer.empty()) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    // Get current process FD table
    FdTable *fd_table = this->GetCurrentProcessFdTable();
    if (fd_table == nullptr) {
        return std::unexpected(FdError::kIoError);
    }

    // Get global entry
    auto entry_result = fd_table->GetGlobalEntry(fd);
    if (!entry_result) {
        return std::unexpected(entry_result.error());
    }
    OpenFileEntry *global_entry = *entry_result;

    // Check if file is opened for reading
    OpenMode mode = static_cast<OpenMode>(global_entry->flags);
    if (!HasMode(mode, OpenMode::kRead) && !HasMode(mode, OpenMode::kReadWrite)) {
        return std::unexpected(FdError::kPermissionDenied);
    }

    // Get file and stream
    File *file = global_entry->file;
    if (file == nullptr || file->stream == nullptr) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    // Read from stream
    auto result = file->stream->Read(buffer);
    if (!result) {
        return std::unexpected(FdError::kIoError);
    }

    // Update offset
    global_entry->offset += *result;

    return *result;
}

FdResult<size_t> FdManager::Write(int fd, std::span<const byte> buffer)
{
    if (buffer.empty()) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    // Get current process FD table
    FdTable *fd_table = this->GetCurrentProcessFdTable();
    if (fd_table == nullptr) {
        return std::unexpected(FdError::kIoError);
    }

    // Get global entry
    auto entry_result = fd_table->GetGlobalEntry(fd);
    if (!entry_result) {
        return std::unexpected(entry_result.error());
    }
    OpenFileEntry *global_entry = *entry_result;

    // Check if file is opened for writing
    OpenMode mode = static_cast<OpenMode>(global_entry->flags);
    if (!HasMode(mode, OpenMode::kWrite) && !HasMode(mode, OpenMode::kReadWrite)) {
        return std::unexpected(FdError::kPermissionDenied);
    }

    // Get file and stream
    File *file = global_entry->file;
    if (file == nullptr || file->stream == nullptr) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    // For append mode, seek to end of file
    if (global_entry->is_append) {
        // TODO: Implement seek to end when VFS supports it
        // For now, just write at current offset
    }

    // Write to stream
    auto result = file->stream->Write(buffer);
    if (!result) {
        return std::unexpected(FdError::kIoError);
    }

    // Update offset
    global_entry->offset += *result;

    return *result;
}

FdTable *FdManager::GetCurrentProcessFdTable()
{
    auto current_thread = SchedulingModule::Get().GetScheduler().Schedule();
    if (current_thread == nullptr) {
        return nullptr;
    }

    auto process = SchedulingModule::Get().GetProcesses().GetProcess(current_thread->owner);
    if (!process) {
        return nullptr;
    }

    return process.value()->fd_table;
}

}  // namespace Fs
