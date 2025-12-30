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

FdTable::FdTable() : open_count_(0), standard_streams_initialized_(false) {}

FdTable::~FdTable()
{
    // TaggedPointer destructor handles cleanup automatically
    LockGuard lock(lock_);
}

FdResult<fd_t> FdTable::AllocateFd(OpenFileEntry *global_entry)
{
    if (global_entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    LockGuard lock(lock_);

    // Find the first available file descriptor
    for (size_t i = 0; i < kMaxFds; ++i) {
        if (!entries_[i].IsValid()) {
            entries_[i] = FdEntry::FromPtr(global_entry);
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
    if (entries_[fd].IsValid()) {
        return std::unexpected(FdError::kFdTableFull);
    }

    // Allocate at the specific index
    entries_[fd] = FdEntry::FromPtr(global_entry);
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
    if (fd <= kStderrFd && entries_[fd].Is<IO::Pipe<4096>>()) {
        return std::unexpected(FdError::kPermissionDenied);
    }

    if (!entries_[fd].IsValid()) {
        return std::unexpected(FdError::kNotOpen);
    }

    // TaggedPointer destructor handles cleanup automatically
    entries_[fd] = FdEntry();
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
    if (!IsValidFd(fd)) {
        return std::unexpected(FdError::kInvalidFd);
    }

    LockGuard lock(lock_);

    if (!entries_[fd].IsValid()) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    // Find the first available file descriptor for the copy
    for (size_t i = 0; i < kMaxFds; ++i) {
        if (!entries_[i].IsValid()) {
            // Copy the entry - TaggedPointer move constructor handles this
            entries_[i] = std::move(entries_[fd]);
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

    // Standard streams are now allocated directly in PrepareProcess with pipes
    // Just mark as initialized
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

    // Get entry using TaggedPointer
    auto *entry = fd_table->GetEntry(fd);
    if (!entry || !entry->IsValid()) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    // Dispatch based on type - close OpenFileEntry if needed
    if (entry->Is<OpenFileEntry *>()) {
        OpenFileEntry *global_entry = entry->As<OpenFileEntry *>();

        // Close global entry
        FdError err = open_file_table_.CloseFile(global_entry);
        if (err != FdError{}) {
            return std::unexpected(err);
        }
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

    // Get entry using TaggedPointer
    auto *entry = fd_table->GetEntry(fd);
    if (!entry || !entry->IsValid()) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    // Dispatch based on type using visitor pattern
    return entry->Visit([&](auto &value) -> FdResult<size_t> {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_pointer_v<T>) {
            // Regular file handling (OpenFileEntry *)
            OpenFileEntry *global_entry = value;
            if (global_entry == nullptr) {
                return std::unexpected(FdError::kBadFileDescriptor);
            }

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
        } else {
            // Standard stream handling (IO::Pipe<4096>)
            auto result = value.Read(buffer);
            if (!result) {
                return std::unexpected(FdError::kIoError);
            }
            return *result;
        }
    });
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

    // Get entry using TaggedPointer
    auto *entry = fd_table->GetEntry(fd);
    if (!entry || !entry->IsValid()) {
        return std::unexpected(FdError::kBadFileDescriptor);
    }

    // Dispatch based on type using visitor pattern
    return entry->Visit([&](auto &value) -> FdResult<size_t> {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_pointer_v<T>) {
            // Regular file handling (OpenFileEntry *)
            OpenFileEntry *global_entry = value;
            if (global_entry == nullptr) {
                return std::unexpected(FdError::kBadFileDescriptor);
            }

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
        } else {
            // Standard stream handling (IO::Pipe<4096>)
            auto result = value.Write(buffer);
            if (!result) {
                return std::unexpected(FdError::kIoError);
            }
            return *result;
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
