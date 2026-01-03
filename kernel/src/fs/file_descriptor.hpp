#ifndef KERNEL_SRC_FS_FILE_DESCRIPTOR_HPP_
#define KERNEL_SRC_FS_FILE_DESCRIPTOR_HPP_

#include <types.h>
#include <data_structures/hash_maps.hpp>
#include <data_structures/ref_count.hpp>
#include <data_structures/tagged_pointer.hpp>
#include <span.hpp>

#include "alkos/sys/fd.h"
#include "fs/costants.hpp"
#include "io/pipe.hpp"
#include "io/stream.hpp"
#include "sync/spinlock.hpp"
#include "vfs/types.hpp"

namespace Fs
{

// ------------------------------
// Forward Declarations
// ------------------------------

class OpenFileTable;
class FileTable;

// ------------------------------
// Error Types
// ------------------------------

enum class FdError {
    kInvalidFd = 1,
    kFdTableFull,
    kBadFileDescriptor,
    kNotOpen,
    kNotFound,
    kAlreadyClosed,
    kPermissionDenied,
    kInvalidArgument,
    kIoError,
};

template <typename T = void>
using FdResult = std::expected<T, FdError>;

// ------------------------------
// File Mode Flags
// -----------------------------

enum class OpenMode : u8 {
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
    return static_cast<OpenMode>(static_cast<u8>(a) | static_cast<u8>(b));
}

constexpr OpenMode operator&(OpenMode a, OpenMode b)
{
    return static_cast<OpenMode>(static_cast<u8>(a) & static_cast<u8>(b));
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
// Seek Whence Options
// ------------------------------

enum class FdSeek : u8 {
    kSet = 0,  // Seek from beginning of file
    kCur = 1,  // Seek from current position
    kEnd = 2   // Seek from end of file
};

// ------------------------------
// Standard File Descriptors
// -----------------------------

inline constexpr fd_t kStdinFd  = kFdStdIn;
inline constexpr fd_t kStdoutFd = kFdStdOut;
inline constexpr fd_t kStderrFd = kFdStdErr;

// ------------------------------
// File Structure
// -----------------------------

/**
 * @brief File represents a file in filesystem
 *
 */
class File : public data_structures::RefCounted<File>
{
    friend class FileTable;

    public:
    u64 size{0};
    u32 mode{0};
    vfs::Path path;

    File() = default;
    ~File();

    private:
    size_t pool_idx_{std::numeric_limits<size_t>::max()};
};

// ------------------------------
// Global File Table
// -----------------------------

/**
 * @brief Global file table
 *
 * Manages all active files.
 */
class FileTable
{
    friend class File;

    public:
    FileTable()  = default;
    ~FileTable() = default;

    FileTable(const FileTable &)            = delete;
    FileTable &operator=(const FileTable &) = delete;
    FileTable(FileTable &&)                 = delete;
    FileTable &operator=(FileTable &&)      = delete;

    FdResult<data_structures::RefPtr<File>> GetOrCreate(const vfs::Path &path);
    File *Find(const vfs::Path &path);
    size_t GetCount() const { return count_; }
    const File *GetFile(size_t index) const { return files_.Get(index); }

    private:
    data_structures::PooledHashMap<File, kMaxActiveFiles> files_{};
    std::atomic_size_t count_{0};
    mutable Spinlock lock_;
};

/**
 * @brief Tagged union ptr holding either a File* or Pipe*
 *
 * - File: Smart (ref-counted), managed by RefCounted
 * - Pipe: NonOwned, owned by Process (for stdin/stdout/stderr)
 */
using FileHandle = data_structures::NonOwningTaggedPtr<File, IO::Pipe<kStdioBufferSize>>;

// ------------------------------
// Global Open File Table Entry
// -----------------------------

/**
 * @brief Global open file table entry
 *
 * This is middle level in three-tier hierarchy.
 * Multiple process file descriptors can reference same entry.
 */
class OpenFileEntry : public data_structures::RefCounted<OpenFileEntry>
{
    friend class OpenFileTable;

    public:
    FileHandle handle;
    u32 flags{0};
    u64 offset{0};
    bool is_append{false};

    OpenFileEntry() = default;
    ~OpenFileEntry();

    bool IsFile() const { return handle.Is<File>(); }
    bool IsPipe() const { return handle.Is<IO::Pipe<kStdioBufferSize>>(); }

    File *GetFile() { return IsFile() ? &handle.As<File>() : nullptr; }

    IO::Pipe<kStdioBufferSize> *GetPipe()
    {
        return IsPipe() ? &handle.As<IO::Pipe<kStdioBufferSize>>() : nullptr;
    }

    private:
    size_t pool_idx_{std::numeric_limits<size_t>::max()};
};

// ------------------------------
// Global Open File Table
// -----------------------------

/**
 * @brief Global open file table
 *
 * Manages all open file entries system-wide.
 */
class OpenFileTable
{
    friend class OpenFileEntry;

    public:
    OpenFileTable() = default;
    ~OpenFileTable();

    OpenFileTable(const OpenFileTable &)            = delete;
    OpenFileTable &operator=(const OpenFileTable &) = delete;
    OpenFileTable(OpenFileTable &&)                 = delete;
    OpenFileTable &operator=(OpenFileTable &&)      = delete;

    FdResult<data_structures::RefPtr<OpenFileEntry>> OpenFile(File *file, OpenMode flags);
    FdResult<data_structures::RefPtr<OpenFileEntry>> OpenPipe(IO::Pipe<kStdioBufferSize> &pipe);

    FdResult<u64> GetOffset(const OpenFileEntry *entry) const;
    FdResult<> SetOffset(OpenFileEntry *entry, u64 offset) const;

    size_t GetOpenCount() const { return count_; }
    const OpenFileEntry *GetEntry(size_t index) const { return entries_.Get(index); }

    private:
    data_structures::PooledHashMap<OpenFileEntry, kMaxOpenFiles> entries_{};
    std::atomic_size_t count_{0};
    mutable Spinlock lock_;
};

// ------------------------------
// Process File Descriptor Table
// -----------------------------

/**
 * @brief Process-local file descriptor table
 *
 * Each process has its own table of file descriptors.
 */
class FdTable
{
    public:
    FdTable()  = default;
    ~FdTable() = default;

    FdTable(const FdTable &)            = delete;
    FdTable &operator=(const FdTable &) = delete;
    FdTable(FdTable &&)                 = delete;
    FdTable &operator=(FdTable &&)      = delete;

    FdResult<fd_t> Allocate(data_structures::RefPtr<OpenFileEntry> global_entry);
    FdResult<fd_t> AllocateAt(data_structures::RefPtr<OpenFileEntry> global_entry, fd_t fd);
    FdResult<> Free(fd_t fd);

    OpenFileEntry *Get(fd_t fd);
    const OpenFileEntry *Get(fd_t fd) const;

    FdResult<fd_t> DuplicateFd(fd_t fd);
    FdResult<fd_t> DuplicateFdTo(fd_t old_fd, fd_t new_fd);

    size_t GetOpenCount() const { return count_; }

    private:
    data_structures::RefPtr<OpenFileEntry> entries_[kMaxFdsPerProcess]{};
    std::atomic_size_t count_{0};
    mutable Spinlock lock_;
};

// ------------------------------
// File Descriptor Manager
// -----------------------------

class FdManager
{
    public:
    FdManager()  = default;
    ~FdManager() = default;

    FdManager(const FdManager &)            = delete;
    FdManager &operator=(const FdManager &) = delete;
    FdManager(FdManager &&)                 = delete;
    FdManager &operator=(FdManager &&)      = delete;

    FdResult<fd_t> Open(const vfs::Path &path, OpenMode flags);
    FdResult<> Close(fd_t fd);
    FdResult<size_t> Read(fd_t fd, std::span<byte> buffer);
    FdResult<size_t> Write(fd_t fd, std::span<const byte> buffer);
    FdResult<ssize_t> Seek(fd_t fd, ssize_t offset, FdSeek whence);
    FdResult<fd_t> Duplicate(fd_t fd);
    FdResult<fd_t> Duplicate(fd_t old_fd, fd_t new_fd);

    FileTable &GetFileTable() { return file_table_; }
    OpenFileTable &GetOpenFileTable() { return open_file_table_; }
    FdTable *GetCurrentProcessFdTable();

    private:
    FileTable file_table_;
    OpenFileTable open_file_table_;
};

}  // namespace Fs

#endif  // KERNEL_SRC_FS_FILE_DESCRIPTOR_HPP_
