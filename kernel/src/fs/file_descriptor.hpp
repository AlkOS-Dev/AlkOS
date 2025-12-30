#ifndef KERNEL_SRC_FS_FILE_DESCRIPTOR_HPP_
#define KERNEL_SRC_FS_FILE_DESCRIPTOR_HPP_

#include <types.h>
#include <span.hpp>

#include "hal/spinlock.hpp"
#include "io/error.hpp"
#include "io/stream.hpp"
#include "mem/types.hpp"
#include "scheduling/process.hpp"
#include "sys/calls/fd.h"
#include "vfs/types.hpp"

namespace Fs
{

// ------------------------------
// Forward Declarations
// ------------------------------

class OpenFileTable;

// ------------------------------
// Error Types
// ------------------------------

enum class FdError {
    kInvalidFd,
    kFdTableFull,
    kBadFileDescriptor,
    kNotOpen,
    kAlreadyClosed,
    kPermissionDenied,
    kInvalidArgument,
    kIoError
};

template <typename T = void>
using FdResult = std::expected<T, FdError>;

// ------------------------------
// File Mode Flags
// -----------------------------

enum class OpenMode : u32 {
    kRead      = 1 << 0,
    kWrite     = 1 << 1,
    kReadWrite = kRead | kWrite,
    kAppend    = 1 << 2,
    kCreate    = 1 << 3,
    kTruncate  = 1 << 4,
    kNonBlock  = 1 << 5,
    kSync      = 1 << 6,
};

constexpr OpenMode operator|(OpenMode a, OpenMode b)
{
    return static_cast<OpenMode>(static_cast<u32>(a) | static_cast<u32>(b));
}

constexpr OpenMode operator&(OpenMode a, OpenMode b)
{
    return static_cast<OpenMode>(static_cast<u32>(a) & static_cast<u32>(b));
}

constexpr bool HasMode(OpenMode flags, OpenMode mode) { return (flags & mode) == mode; }

// ------------------------------
// Buffering Modes for libc FILE
// ------------------------------

enum class BufferMode : u8 {
    kNone = kFdBufferNone,  // No buffering
    kLine = kFdBufferLine,  // Line buffering (flush on newline)
    kFull = kFdBufferFull,  // Full buffering (flush when buffer full)
};

// ------------------------------
// Standard Stream Types
// -----------------------------

enum class StandardStreamType { kStdin, kStdout, kStderr };

// ------------------------------
// File Structure
// ------------------------------

/**
 * @brief File represents a file in filesystem
 */
struct File {
    u64 size;       // File size in bytes
    u32 mode;       // File mode (permissions and type)
    u64 ref_count;  // Number of open file entries referencing this file

    // Underlying I/O stream (wraps VFS operations)
    IO::IStream *stream;
    vfs::MountPoint *mount_point;
    vfs::Path rel_path;

    // Standard stream flag
    bool is_standard_stream;
};

// ------------------------------
// Global Open File Table Entry
// ------------------------------

/**
 * @brief Global open file table entry
 * This is middle level in three-tier hierarchy
 * Multiple process file descriptors can reference same entry
 */
struct OpenFileEntry {
    u32 flags;
    u64 offset;
    u32 ref_count;
    File *file;

    // Access control
    bool is_append;
};

// ------------------------------
// Open File Handle with RAII
// -----------------------------

/**
 * @brief RAII wrapper for OpenFileEntry with automatic resource management
 * Manages reference counting and cleanup automatically
 */
class OpenFileHandle
{
    public:
    OpenFileHandle() noexcept : entry_(nullptr), own_table_(nullptr) {}

    explicit OpenFileHandle(OpenFileEntry *entry, OpenFileTable *table) noexcept
        : entry_(entry), own_table_(table)
    {
        if (entry_ != nullptr) {
            entry_->ref_count++;
        }
    }

    ~OpenFileHandle() noexcept
    {
        if (entry_ != nullptr && own_table_ != nullptr && entry_->ref_count > 0) {
            entry_->ref_count--;
            if (entry_->ref_count == 0 && entry_->file != nullptr) {
                entry_->file->ref_count--;
                if (entry_->file->ref_count == 0 && entry_->file->stream != nullptr) {
                    delete entry_->file->stream;
                    entry_->file->stream = nullptr;
                }
            }
        }
    }

    OpenFileHandle(const OpenFileHandle &other) noexcept
        : entry_(other.entry_), own_table_(other.own_table_)
    {
        if (entry_ != nullptr) {
            entry_->ref_count++;
        }
    }

    OpenFileHandle(OpenFileHandle &&other) noexcept
        : entry_(other.entry_), own_table_(other.own_table_)
    {
        other.entry_     = nullptr;
        other.own_table_ = nullptr;
    }

    OpenFileHandle &operator=(const OpenFileHandle &other) noexcept
    {
        if (this != &other) {
            this->~OpenFileHandle();

            entry_     = other.entry_;
            own_table_ = other.own_table_;

            if (entry_ != nullptr) {
                entry_->ref_count++;
            }
        }
        return *this;
    }

    OpenFileHandle &operator=(OpenFileHandle &&other) noexcept
    {
        if (this != &other) {
            this->~OpenFileHandle();

            entry_     = other.entry_;
            own_table_ = other.own_table_;

            other.entry_     = nullptr;
            other.own_table_ = nullptr;
        }
        return *this;
    }

    explicit operator bool() const noexcept { return entry_ != nullptr; }

    OpenFileEntry *Get() const noexcept { return entry_; }
    OpenFileEntry *operator->() const noexcept { return entry_; }
    OpenFileEntry &operator*() const noexcept { return *entry_; }

    private:
    OpenFileEntry *entry_;
    OpenFileTable *own_table_;
};

// ------------------------------
// Process File Descriptor Table
// ------------------------------

/**
 * @brief Process-local file descriptor table
 * Each process has its own table of file descriptors
 */
class FdTable
{
    public:
    static constexpr size_t kMaxFds = 256;

    struct FdEntry {
        OpenFileEntry *global_entry;
        u32 fd_flags;
        bool is_standard_stream;
    };

    FdTable();
    ~FdTable();

    FdResult<fd_t> AllocateFd(OpenFileEntry *global_entry);
    FdResult<fd_t> AllocateFdAt(OpenFileEntry *global_entry, fd_t fd);
    FdResult<> FreeFd(fd_t fd);
    FdResult<OpenFileEntry *> GetGlobalEntry(fd_t fd);
    bool IsValidFd(fd_t fd) const;
    FdResult<fd_t> DuplicateFd(fd_t fd);
    size_t GetOpenCount() const { return open_count_; }
    bool IsStandardStreamInitialized() const;
    void SetStandardStreamsInitialized(bool initialized);
    FdEntry *GetEntries() { return entries_; }

    private:
    FdEntry entries_[kMaxFds];
    size_t open_count_;
    bool standard_streams_initialized_;
    mutable arch::Spinlock lock_;
};

// ------------------------------
// Global File Table
// ------------------------------

/**
 * @brief Global file table
 * Manages all files system-wide
 */
class FileTable
{
    public:
    static constexpr size_t kMaxFiles = 1024;

    FileTable();
    ~FileTable();

    FdResult<File *> GetOrCreate(const vfs::Path &path);
    FdResult<File *> CreateStandardStream(StandardStreamType);
    FdResult<> Release(File *file);
    std::optional<File *> Find(const vfs::Path &path);

    private:
    std::optional<File *> FindUnlocked(const vfs::Path &path) const;

    File files_[kMaxFiles];
    size_t count_;
    mutable arch::Spinlock lock_;
};

// ------------------------------
// Global Open File Table
// ------------------------------

/**
 * @brief Global open file table
 * Manages all open file entries system-wide
 */
class OpenFileTable
{
    public:
    static constexpr size_t kMaxOpenFiles = 512;

    OpenFileTable();
    ~OpenFileTable();

    FdError OpenFile(File *inode, OpenMode flags, OpenFileEntry **entry_out);
    FdError CloseFile(OpenFileEntry *entry);
    FdError GetOffset(OpenFileEntry *entry, u64 *offset_out);
    FdError SetOffset(OpenFileEntry *entry, u64 offset);

    private:
    OpenFileEntry entries_[kMaxOpenFiles];
    size_t open_count_;
    mutable arch::Spinlock lock_;
};

// ------------------------------
// Standard File Descriptors
// -----------------------------

inline constexpr fd_t kStdinFd  = kFdStdIn;
inline constexpr fd_t kStdoutFd = kFdStdOut;
inline constexpr fd_t kStderrFd = kFdStdErr;

// ------------------------------
// File Descriptor Manager
// -----------------------------

/**
 * @brief Main file descriptor manager
 * Coordinates three-tier hierarchy
 */
class FdManager
{
    public:
    FdManager();
    ~FdManager();

    FdResult<> InitializeStandardStreams(FdTable &fd_table);
    FdResult<fd_t> Open(const vfs::Path &path, OpenMode flags);
    FdResult<> Close(int fd);
    FdResult<size_t> Read(int fd, std::span<byte> buffer);
    FdResult<size_t> Write(int fd, std::span<const byte> buffer);
    FileTable &GetFileTable() { return file_table_; }
    OpenFileTable &GetOpenFileTable() { return open_file_table_; }
    FdTable *GetCurrentProcessFdTable();

    private:
    FileTable file_table_;
    OpenFileTable open_file_table_;
    bool standard_streams_initialized_;
};

}  // namespace Fs

#endif  // KERNEL_SRC_FS_FILE_DESCRIPTOR_HPP_
